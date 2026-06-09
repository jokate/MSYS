// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/YSBlueprintFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "NiagaraFunctionLibrary.h"
#include "YSBattleActor.h"
#include "YSDeveloperSettings.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Character/YSCharacterBase.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "Character/Components/YSLockOnComponent.h"
#include "General/YSGameplayTag.h"

float UYSBlueprintFunctionLibrary::GetFinalDamage(const UYSCharacterAttributeSetBase* Owner,
                                                  const UYSCharacterAttributeSetBase* Target, const FName& SkillID)
{
	const FYSDamageInfo* DamageInfo = UYSDeveloperSettings::GetDamageInfo(SkillID);
	
	if (IsValid(Owner) == false || IsValid(Target) == false )
		return 0.f;

	// 데미지 정보가 없으면 의미 X
	if ( DamageInfo == nullptr )
		return 0.f;
	
	// 당장 생각하는 부분은 (공격 데미지 * (공격 계수)  ) * ( 1 - DefenceRate ) + 고정 데미지
	float FinalDamage = ( Owner->GetAttackDmg() * DamageInfo->DamageMultiplier ) * ( 1 - Target->GetDefenseRate()) + DamageInfo->AdditiveTrueDamage;

	// 차후 데미지 증가 버프 혹은 다른 것들이 추가될 경우 해당 부분에 대해서 확장하기로 합니다.
	
	return FinalDamage;
}

void UYSBlueprintFunctionLibrary::SendHitEventToTarget(AActor* Instigator, AActor* Target, const FName& SkillID)
{
	const FYSDamageInfo* DamageInfo = UYSDeveloperSettings::GetDamageInfo(SkillID);

	// 데미지 정보가 없으면 의미 X
	if ( DamageInfo == nullptr )
		return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	
	if ( IsValid(ASC) == false || IsValid(TargetASC) == false )
		return;
	
	float FinalDamage = GetFinalDamage(ASC->GetSet<UYSCharacterAttributeSetBase>(), ASC->GetSet<UYSCharacterAttributeSetBase>(), SkillID);
	
	FGameplayEventData EventData;
	EventData.Instigator = Instigator;
	EventData.Target = Target;
	EventData.EventMagnitude = FinalDamage;

	TargetASC->HandleGameplayEvent(DamageInfo->HitTag, &EventData);
}

void UYSBlueprintFunctionLibrary::ProcessHits(
	AActor* Instigator,
	const TArray<FHitResult>& RawHits,
	const FName& DamageRow,
	TFunctionRef<void(const TArray<FHitResult>&)> OnValidated,
	bool bForceLockonOnJustAvoid)
{
	if (!IsValid(Instigator))
		return;

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);
	if (!IsValid(OwnerASC))
		return;

	const UYSCharacterAttributeSetBase* OwnerAttribute = OwnerASC->GetSet<UYSCharacterAttributeSetBase>();
	if (!IsValid(OwnerAttribute))
		return;

	IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Instigator);
	if (TeamAgent == nullptr)
		return;

	TArray<FHitResult> ValidHits;

	for (const FHitResult& HitResult : RawHits)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!IsValid(HitActor))
			continue;

		if (TeamAgent->GetTeamAttitudeTowards(*HitActor) == ETeamAttitude::Friendly)
			continue;

		IYSBattleActor* BattleActor = Cast<IYSBattleActor>(HitActor);
		if (BattleActor == nullptr || BattleActor->IsDead())
			continue;

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!IsValid(TargetASC))
			continue;

		const UYSCharacterAttributeSetBase* TargetAttribute = TargetASC->GetSet<UYSCharacterAttributeSetBase>();
		if (!IsValid(TargetAttribute))
			continue;

		// JustAvoid 윈도우 처리
		if (TargetASC->HasMatchingGameplayTag(YSTags::JustAvoid_Window))
		{
			FGameplayEventData EventData;
			EventData.Instigator = Instigator;
			EventData.ContextHandle.AddHitResult(HitResult);

			if (bForceLockonOnJustAvoid)
			{
				if (UYSLockOnComponent* LockOn = UYSLockOnComponent::Get(HitActor))
				{
					LockOn->ForceSetLockOn(Instigator);
				}
			}

			TargetASC->HandleGameplayEvent(YSTags::Event_JustAvoid, &EventData);
			continue;
		}

		// 무적 상태 스킵
		if (TargetASC->HasMatchingGameplayTag(YSTags::Invincible))
			continue;

		// 데미지 이벤트 전송
		SendHitEventToTarget(Instigator, HitActor, DamageRow);
		
		SpawnEffects(Instigator, DamageRow, HitResult.ImpactPoint, HitResult.ImpactNormal.ToOrientationRotator());
		
		ValidHits.Add(HitResult);
	}

	if (ValidHits.Num() > 0)
	{
		OnValidated(ValidHits);
	}
}

void UYSBlueprintFunctionLibrary::SpawnEffects(UObject* WorldContextObject, const FName& DamageRow,
	const FVector& Location, const FRotator& Rotation)
{
	const FYSDamageInfo* DamageInfo = UYSDeveloperSettings::GetDamageInfo(DamageRow);
	
	if ( DamageInfo == nullptr )
	{
		return;
	}
	
	
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(WorldContextObject, DamageInfo->HitEffect, Location, Rotation);
	
	
	UWorld* World = WorldContextObject->GetWorld();
	
	if ( IsValid(World) == false )
	{
		return;
	}
	
	APlayerController* PC = World->GetFirstPlayerController();
	
	// 카메라 셰이크도 함께 처리
	if (IsValid(DamageInfo->HitCameraShake) && IsValid(PC))
	{
		APlayerCameraManager* CameraManager = PC->PlayerCameraManager;
		if (IsValid(CameraManager))
		{
			CameraManager->StartCameraShake(DamageInfo->HitCameraShake);
		}
	}
}

FRotator UYSBlueprintFunctionLibrary::GetAbilityEventRotation(EYSDirectionPolicy DirectionPolicy,
                                                              UYSGameplayAbility* OwningAbility, const FName& SocketName)
{
	AActor* OwnerActor = OwningAbility->GetOwningActorFromActorInfo();
	
	if ( IsValid(OwnerActor) == false )
	{
		return FRotator();
	}
	
	switch (DirectionPolicy)
	{
	case EYSDirectionPolicy::UseSocketRotation:
		{
			AYSCharacterBase* Character = Cast<AYSCharacterBase>(OwnerActor);
			if (IsValid(Character) && IsValid(Character->GetMesh()))
			{
				if (SocketName != NAME_None)
					return Character->GetMesh()->GetSocketRotation(SocketName);
			}
			return OwnerActor->GetActorRotation();
		}

	case EYSDirectionPolicy::UseControlRotation:
		{
			if (const AController* Controller = OwnerActor->GetInstigatorController())
				return Controller->GetControlRotation();

			return OwnerActor->GetActorRotation();
		}

	case EYSDirectionPolicy::UseTowardLockOnTarget:
		{
			if (UYSLockOnComponent* LockOn = UYSLockOnComponent::Get(OwnerActor))
			{
				if (AActor* Target = LockOn->GetCurrentTarget())
				{
					const FVector Dir = (Target->GetActorLocation() - OwnerActor->GetActorLocation()).GetSafeNormal();
					return Dir.Rotation();
				}
			}
			return OwnerActor->GetActorRotation();
		}

	case EYSDirectionPolicy::UseTowardPlaybackTarget:
		{
			if (const UYSAbilityPlaybackBase* Playback = OwningAbility->GetCurrentPlayback())
			{
				if (AActor* Target = Playback->GetCurrentPlaybackTarget())
				{
					const FVector Dir = (Target->GetActorLocation() - OwnerActor->GetActorLocation()).GetSafeNormal();
					return Dir.Rotation();
				}
			}
			return OwnerActor->GetActorRotation();
		}

	case EYSDirectionPolicy::UseActorForwardVector:
	default:
		return OwnerActor->GetActorRotation();
	}
}

FVector UYSBlueprintFunctionLibrary::GetAbilityEventPosition(EYSPositionPolicy PositionPolicy,
	UYSGameplayAbility* OwningAbility, const FName& SocketName, const FVector& RelativeOffset)
{
	AActor* OwnerActor = OwningAbility->GetOwningActorFromActorInfo();
	
	if ( IsValid(OwnerActor) == false )
	{
		return FVector::ZeroVector;
	}
	
	switch (PositionPolicy)
	{
	case EYSPositionPolicy::UseSocket:
		{
			AYSCharacterBase* Character = Cast<AYSCharacterBase>(OwnerActor);
			if (IsValid(Character) && IsValid(Character->GetMesh()) && SocketName != NAME_None)
			{
				return Character->GetMesh()->GetSocketLocation(SocketName);
			}
			return OwnerActor->GetActorLocation();
		}

	case EYSPositionPolicy::UseRelativeOffset:
		return OwnerActor->GetActorTransform().TransformPosition(RelativeOffset);

	case EYSPositionPolicy::UseActorLocation:
	default:
		return OwnerActor->GetActorLocation();
	}
}
