// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/YSSwitchEntryForm.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "MotionWarpingComponent.h"
#include "TimerManager.h"
#include "YSBattleActor.h"
#include "Ability/GameplayEffect/YSGE_SwitchEntryGroggy.h"
#include "Framework/YSGameModeBase.h"
#include "General/YSGameplayTag.h"
#include "Library/YSBlueprintFunctionLibrary.h"
#include "Subsystem/YSSquadSubsystem.h"
#include "Targeting/YSTargetingShape.h"

bool FYSSwitchEntryForm::CanUse(const AActor* Instigator, const AActor* Target) const
{
	const bool bNeedsTarget = TargetRule == EYSSwitchEntryTargetRule::CurrentTarget || MaxTargetDistance > 0.f;

	if ( bNeedsTarget == false )
	{
		return true;
	}

	if ( IsValid(Instigator) == false || IsValid(Target) == false )
	{
		return false;
	}

	return MaxTargetDistance <= 0.f || FVector::Dist2D(Instigator->GetActorLocation(), Target->GetActorLocation()) <= MaxTargetDistance;
}

void FYSSwitchEntryForm::CollectTargets(AActor* Instigator, AActor* Target, TArray<AActor*>& OutTargets) const
{
	const FYSTargetingShape* HitShape = Shape.GetPtr<FYSTargetingShape>();

	if ( IsValid(Instigator) == false || HitShape == nullptr )
	{
		return;
	}

	TArray<AActor*> InShape;

	if ( HitShape->CollectActors(Instigator->GetWorld(), Instigator->GetActorLocation(), Instigator->GetActorForwardVector(), Instigator, InShape) == false )
	{
		UE_LOG(LogTemp, Warning, TEXT("[SwitchEntry] 판정 도형이 판정을 지원하지 않는다 (시전자 기준 도형만 된다)."));
		return;
	}

	// 1체 규칙은 관통 없이 현재 타겟만.
	if ( TargetRule == EYSSwitchEntryTargetRule::CurrentTarget )
	{
		InShape = InShape.Contains(Target) ? TArray<AActor*>{ Target } : TArray<AActor*>();
	}

	// 판정은 한 번이라 이번 판정 안에서만 저스트 회피 GE 를 대상당 한 번 준다.
	TSet<TWeakObjectPtr<AActor>> JustAvoidGranted;

	for ( AActor* Actor : InShape )
	{
		// 아군 · 사망 · 저스트 회피 창 · 무적 검사는 트레이스 · DirectHit 과 같은 함수를 거친다.
		if ( UYSBlueprintFunctionLibrary::AdmitHit(Instigator, Actor, &JustAvoidGranted) )
		{
			OutTargets.Add(Actor);
		}
	}
}

void FYSSwitchEntryForm::ApplyToTargets(AActor* Instigator, const TArray<AActor*>& Targets) const
{
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);

	if ( IsValid(SourceASC) == false )
	{
		return;
	}

	const UGameInstance* GameInstance = Instigator->GetGameInstance();
	const UYSSquadSubsystem* Squad = GameInstance ? GameInstance->GetSubsystem<UYSSquadSubsystem>() : nullptr;

	for ( AActor* Target : Targets )
	{
		const FVector HitLocation = Target->GetActorLocation();
		const FVector HitDirection = (HitLocation - Instigator->GetActorLocation()).GetSafeNormal2D();

		if ( DamageRow.IsNone() == false )
		{
			const FHitResult HitResult(Target, Cast<UPrimitiveComponent>(Target->GetRootComponent()), HitLocation, -HitDirection);

			UYSBlueprintFunctionLibrary::SendHitEventToTarget(Instigator, Target, DamageRow);
			UYSBlueprintFunctionLibrary::SpawnEffects(Instigator, DamageRow, HitLocation, HitDirection.ToOrientationRotator());
			UYSBlueprintFunctionLibrary::ApplyHitEffects(Instigator, Instigator, Target, DamageRow, HitResult);
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

		if ( GroggyDamage <= 0.f || IsValid(TargetASC) == false )
		{
			continue;
		}

		const float Scale = Squad ? Squad->GetSwitchEffectScale(Target) : 1.f;

		FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
		Context.AddInstigator(Instigator, Instigator);

		FGameplayEffectSpec Spec(GetDefault<UYSGE_SwitchEntryGroggy>(), Context);
		Spec.SetSetByCallerMagnitude(YSTags::Data_Groggy_SwitchEntryDamage, GroggyDamage * Scale);

		SourceASC->ApplyGameplayEffectSpecToTarget(Spec, TargetASC);
	}

	// 피해 행의 임팩트 칸이 등급이다. 맞은 대상 수와 상관없이 진입기 1회에 한 번.
	if ( Targets.Num() > 0 && DamageRow.IsNone() == false )
	{
		UYSBlueprintFunctionLibrary::PlayHitImpact(Instigator, Targets, DamageRow);
	}
}

void FYSSwitchEntryForm::SetWarp(AActor* Instigator, const AActor* Target) const
{
	if ( Move != EYSSwitchEntryMove::WarpToTarget || WarpTargetName.IsNone() || IsValid(Instigator) == false || IsValid(Target) == false )
	{
		return;
	}

	UMotionWarpingComponent* MotionWarping = Instigator->FindComponentByClass<UMotionWarpingComponent>();

	if ( IsValid(MotionWarping) == false )
	{
		return;
	}

	const FVector From = Instigator->GetActorLocation();
	FVector ToTarget = Target->GetActorLocation() - From;
	ToTarget.Z = 0.f;

	const FVector Direction = ToTarget.GetSafeNormal();
	const float Travel = FMath::Clamp(ToTarget.Size() - WarpStopDistance, 0.f, WarpMaxDistance);

	MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(WarpTargetName, From + Direction * Travel, Direction.Rotation());
}

void FYSSwitchEntryForm::ReleaseWarp(AActor* Instigator) const
{
	if ( Move != EYSSwitchEntryMove::WarpToTarget || WarpTargetName.IsNone() || IsValid(Instigator) == false )
	{
		return;
	}

	if ( UMotionWarpingComponent* MotionWarping = Instigator->FindComponentByClass<UMotionWarpingComponent>() )
	{
		MotionWarping->RemoveWarpTarget(WarpTargetName);
	}
}
