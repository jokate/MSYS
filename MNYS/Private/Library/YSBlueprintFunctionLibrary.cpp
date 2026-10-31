// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/YSBlueprintFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "YSAbilitySystemComponent.h"
#include "YSBattleActor.h"
#include "YSDeveloperSettings.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "AttackableActor/YSAttackableBase.h"
#include "AttackableActor/YSTelegraphActor.h"
#include "AttackableActor/DamageEffect/YSDamageEffect.h"
#include "Character/YSCharacterBase.h"
#include "Character/YSMonster.h"
#include "Character/AttributeSet/YSGroggyAttributeSet.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "Camera/PlayerCameraManager.h"
#include "Character/Components/YSCameraManageComponent.h"
#include "Character/Components/YSTargetingComponent.h"
#include "Framework/YSGameModeBase.h"
#include "General/YSDefine.h"
#include "General/YSGameplayTag.h"
#include "Interface/YSDamageProxy.h"
#include "Interface/YSLinkedActor.h"
#include "Subsystem/YSObjectPoolingSubsystem.h"

namespace
{
	float GetRelativeEvaluationMultiplier(const UYSCharacterAttributeSetBase* Owner, const UYSCharacterAttributeSetBase* Target)
	{
		const UAbilitySystemComponent* OwnerASC = Owner->GetOwningAbilitySystemComponent();
		if (IsValid(OwnerASC) == false || OwnerASC->HasMatchingGameplayTag(YSTags::Passive_RelativeEvaluation) == false)
			return 1.f;

		// HP 비율 비교를 교차곱으로 해서 MaxHp 0 나눗셈을 피한다
		const bool bTargetHealthier = Owner->GetHp() * Target->GetMaxHp() < Target->GetHp() * Owner->GetMaxHp();
		return bTargetHealthier ? GetDefault<UYSDeveloperSettings>()->RelativeEvaluationBonus : 1.f;
	}
}

float UYSBlueprintFunctionLibrary::GetFinalDamage(const UYSCharacterAttributeSetBase* Owner,
                                                  const UYSCharacterAttributeSetBase* Target, const FName& SkillID, float EffectScale)
{
	const FYSDamageInfo* DamageInfo = UYSDeveloperSettings::GetDamageInfo(SkillID);

	if (IsValid(Owner) == false || IsValid(Target) == false )
		return 0.f;

	if ( DamageInfo == nullptr )
		return 0.f;

	float BaselineDamage = DamageInfo->DamageType == EYSDamageType::Melee ? Owner->GetMeleeAttackDmg() : Owner->GetRangedAttackDmg();

	// (공격 데미지 * 공격 계수) * (1 - DefenceRate) * 받는 피해 배율 * 상대평가 * 대상 상태 배율 * 효과 배율 + 고정 데미지. 고정 데미지는 배율을 받지 않는다.
	float ScaledDamage = ( BaselineDamage * DamageInfo->DamageMultiplier ) * ( 1 - Target->GetDefenseRate()) * Target->GetIncomingDamageMultiplier();
	ScaledDamage *= GetRelativeEvaluationMultiplier(Owner, Target);

	if ( const UAbilitySystemComponent* TargetASC = Target->GetOwningAbilitySystemComponent() )
	{
		for ( const TPair<FGameplayTag, float>& StateMultiplier : DamageInfo->TargetStateMultipliers )
		{
			if ( TargetASC->HasMatchingGameplayTag(StateMultiplier.Key) )
			{
				ScaledDamage *= StateMultiplier.Value;
			}
		}
	}

	return ScaledDamage * EffectScale + DamageInfo->AdditiveTrueDamage;
}

bool UYSBlueprintFunctionLibrary::AdmitHit(AActor* Instigator, AActor* Target, TSet<TWeakObjectPtr<AActor>>* JustAvoidGranted)
{
	if ( IsValid(Instigator) == false || IsValid(Target) == false )
		return false;

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);
	if ( IsValid(OwnerASC) == false || IsValid(OwnerASC->GetSet<UYSCharacterAttributeSetBase>()) == false )
		return false;

	const IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Instigator);
	if ( TeamAgent == nullptr || TeamAgent->GetTeamAttitudeTowards(*Target) == ETeamAttitude::Friendly )
		return false;

	const IYSBattleActor* BattleActor = Cast<IYSBattleActor>(Target);
	if ( BattleActor == nullptr || BattleActor->IsDead() )
		return false;

	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	if ( IsValid(TargetASC) == false || IsValid(TargetASC->GetSet<UYSCharacterAttributeSetBase>()) == false )
		return false;

	// 저스트 회피 창: 피해 대신 버프를 부여한다. 이 시점의 버프가 이펙트 출력을 담당한다.
	if ( TargetASC->HasMatchingGameplayTag(YSTags::JustAvoid_Window) )
	{
		// 이 창에서 이미 줬으면 다시 걸지 않는다 (Add 가 이미 있었다고 알려 준다).
		bool bAlreadyGranted = false;
		if ( JustAvoidGranted != nullptr )
		{
			JustAvoidGranted->Add(Target, &bAlreadyGranted);
		}

		const UYSDeveloperSettings* DevSetting = GetDefault<UYSDeveloperSettings>();

		if ( bAlreadyGranted == false && IsValid(DevSetting) && DevSetting->JustAvoidGameplayEffect != nullptr )
		{
			FGameplayEffectContextHandle EffectContextHandle = OwnerASC->MakeEffectContext();
			EffectContextHandle.AddInstigator(Instigator, Instigator);
			FGameplayEffectSpec Spec(DevSetting->JustAvoidGameplayEffect.GetDefaultObject(), EffectContextHandle);
			Spec.AddDynamicAssetTag(YSTags::Event_JustAvoid);

			OwnerASC->ApplyGameplayEffectSpecToTarget(Spec, TargetASC);
		}

		return false;
	}

	// 창이 닫혔으면 다음 창에서 다시 받을 수 있게 기록을 지운다.
	if ( JustAvoidGranted != nullptr )
	{
		JustAvoidGranted->Remove(Target);
	}

	return TargetASC->HasMatchingGameplayTag(YSTags::Invincible) == false;
}

void UYSBlueprintFunctionLibrary::SendHitEventToTarget(AActor* Instigator, AActor* Target, const FName& SkillID, float EffectScale)
{
	const FYSDamageInfo* DamageInfo = UYSDeveloperSettings::GetDamageInfo(SkillID);

	if ( DamageInfo == nullptr )
		return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	if ( IsValid(ASC) == false || IsValid(TargetASC) == false )
		return;

	// 무적이면 피해가 통째로 버려진다(AttributeSet). 그로기 피해 · 처치 · 피격 모션(HitTag)도 보내지 않는다.
	if ( TargetASC->HasMatchingGameplayTag(YSTags::Invincible) )
		return;

	const UYSCharacterAttributeSetBase* TargetSet = TargetASC->GetSet<UYSCharacterAttributeSetBase>();
	const bool bWasAlive = TargetSet != nullptr && TargetSet->GetHp() > 0.f;

	float FinalDamage = GetFinalDamage(ASC->GetSet<UYSCharacterAttributeSetBase>(), TargetSet, SkillID, EffectScale);
	TargetASC->SetNumericAttributeBase(UYSCharacterAttributeSetBase::GetIncomingDamageAttribute(), FinalDamage);

	if ( bWasAlive && TargetSet->GetHp() <= 0.f )
	{
		FGameplayEventData EventData;
		EventData.Instigator = Instigator;
		EventData.Target = Target;
		ASC->HandleGameplayEvent(YSTags::Event_OnKill, &EventData);
	}

	const bool bStillAlive = TargetSet != nullptr && TargetSet->GetHp() > 0.f;

	if ( DamageInfo->GroggyDamage > 0.f && bStillAlive )
	{
		UYSGroggyAttributeSet::ApplyGroggyDamage(Instigator, Target, DamageInfo->GroggyDamage * EffectScale);
	}

	FGameplayEventData EventData;
	EventData.Instigator = Instigator;
	EventData.Target = Target;
	EventData.EventMagnitude = FinalDamage;

	TargetASC->HandleGameplayEvent(DamageInfo->HitTag, &EventData);
}

FName UYSBlueprintFunctionLibrary::ResolveDamageRow(const AActor* Target, const FName& DamageRow)
{
	const FYSDamageInfo* DamageInfo = UYSDeveloperSettings::GetDamageInfo(DamageRow);
	const AYSMonster* Monster = Cast<AYSMonster>(Target);

	if ( DamageInfo == nullptr || DamageInfo->LightGradeDamageRow.IsNone() || IsValid(Monster) == false || Monster->StaggerGrade == EYSStaggerGrade::Immovable )
	{
		return DamageRow;
	}

	return DamageInfo->LightGradeDamageRow;
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
}

void UYSBlueprintFunctionLibrary::PlayHitImpact(AActor* Instigator, const TArray<AActor*>& Targets, const FName& DamageRow)
{
	const FYSDamageInfo* DamageInfo = UYSDeveloperSettings::GetDamageInfo(DamageRow);

	if ( DamageInfo == nullptr )
	{
		return;
	}

	PlayImpact(Instigator, Targets, DamageInfo->Impact);
}

void UYSBlueprintFunctionLibrary::PlayImpact(AActor* Instigator, const TArray<AActor*>& Targets, EYSImpactGrade Grade)
{
	UWorld* World = IsValid(Instigator) ? Instigator->GetWorld() : nullptr;
	const FYSImpactProfile* Profile = GetDefault<UYSDeveloperSettings>()->GetImpactProfile(Grade);

	if ( World == nullptr || Profile == nullptr )
	{
		return;
	}

	if ( Profile->TimeDuration > 0.f )
	{
		if ( AYSGameModeBase* GameMode = World->GetAuthGameMode<AYSGameModeBase>() )
		{
			GameMode->StartTimedDilation(Profile->TimeDilation, Profile->TimeDuration, Profile->TimeScope, Instigator, Targets);
		}
	}

	APlayerController* PC = World->GetFirstPlayerController();

	if ( IsValid(PC) == false )
	{
		return;
	}

	if ( IsValid(Profile->ShakeClass) && IsValid(PC->PlayerCameraManager) )
	{
		PC->PlayerCameraManager->StartCameraShake(Profile->ShakeClass, Profile->ShakeScale);
	}

	if ( Profile->ZoomDuration > 0.f )
	{
		if ( UYSCameraManageComponent* CameraManage = UYSCameraManageComponent::Get(PC->GetPawn()) )
		{
			CameraManage->PlayImpactZoom(*Profile);
		}
	}
}

AActor* UYSBlueprintFunctionLibrary::AcquirePooledActor(UWorld* World, TSubclassOf<AActor> ActorClass,
	const FTransform& SpawnTransform)
{
	UYSObjectPoolingSubsystem* ObjectPooling = UYSObjectPoolingSubsystem::Get(World);
	if ( IsValid(ObjectPooling) == false || ActorClass == nullptr )
	{
		return nullptr;
	}

	if ( AActor* PooledActor = ObjectPooling->GetPooledActor(ActorClass) )
	{
		PooledActor->SetActorTransform(SpawnTransform);
		return PooledActor;
	}

	AActor* SpawnedActor = World->SpawnActorDeferred<AActor>(ActorClass, SpawnTransform);
	if ( IsValid(SpawnedActor) == false )
	{
		return nullptr;
	}

	ObjectPooling->AddPooledActor(SpawnedActor);

	// 컴포넌트 등록과 BeginPlay 는 여기서 끝난다. 초기화·활성화보다 반드시 먼저여야 한다.
	SpawnedActor->FinishSpawning(SpawnTransform);

	// FinishSpawning 이 스폰 콜리전 정책이나 컨스트럭션 스크립트로 액터를 파괴할 수 있다.
	return IsValid(SpawnedActor) ? SpawnedActor : nullptr;
}

namespace
{
	AActor* SpawnAtTransform(UWorld* World, TSubclassOf<AActor> ActorClass, bool bAttachToActor, const FTransform& SpawnTransform,
		AActor* OwnerActor, AActor* AttachParent, const TSharedPtr<FYSAbilityHitContext>& HitContext)
	{
		AActor* PooledActor = UYSBlueprintFunctionLibrary::AcquirePooledActor(World, ActorClass, SpawnTransform);
		if (IsValid(PooledActor) == false)
			return nullptr;

		AActor* Instigator = UYSBlueprintFunctionLibrary::ResolveDamageInstigator(OwnerActor);

		if (bAttachToActor && IsValid(AttachParent))
		{
			PooledActor->AttachToActor(AttachParent, FAttachmentTransformRules::KeepWorldTransform);
		}
		
		if ( IYSSpawnInitializable* Initializable = Cast<IYSSpawnInitializable>(PooledActor) )
		{
			if ( Initializable->OnSpawnInitialize(OwnerActor, Instigator, HitContext) == false )
			{
				UYSObjectPoolingSubsystem::ReturnToPool(PooledActor);
				return nullptr;
			}

			// 활성화가 곧장 반환·파괴로 이어질 수 있어(bTraceOnce 등) SetPoolActive 전에 등록한다.
			if ( HitContext.IsValid() && PooledActor->Implements<UYSLinkedActor>() )
			{
				HitContext->RegisterLinkedActor(PooledActor);
			}

			Initializable->SetPoolActive(true);
		}
		
		return PooledActor;
	}
}

AActor* UYSBlueprintFunctionLibrary::SpawnByConfig(UObject* WorldContext, const FYSSpawnActorConfig& Config,
	const FYSTransformPolicyContext& PolicyContext, AActor* AttachParent, const TSharedPtr<FYSAbilityHitContext>& HitContext)
{
	if (IsValid(WorldContext) == false)
		return nullptr;

	UWorld* World = WorldContext->GetWorld();
	if (IsValid(World) == false || Config.ActorClass == nullptr )
		return nullptr;

	// 자리는 요청한 때 정한다. 예고 표시와 같은 자리에 나와야 한다.
	const FTransform SpawnTransform = CalculateSpawnTransform(WorldContext, Config, PolicyContext);

	if ( Config.SpawnDelay <= 0.f )
	{
		return SpawnAtTransform(World, Config.ActorClass, Config.bAttachToActor, SpawnTransform, PolicyContext.OwnerActor, AttachParent, HitContext);
	}

	const bool bCancelOnEnd = Config.bCancelOnAbilityEnd && HitContext.IsValid();
	if ( bCancelOnEnd && HitContext->IsOwnerActive() == false )
		return nullptr;

	const AYSAttackableBase* AttackableCDO = Cast<AYSAttackableBase>(Config.ActorClass->GetDefaultObject());
	const TWeakObjectPtr<AYSTelegraphActor> WeakTelegraph = IsValid(AttackableCDO) ? AttackableCDO->SpawnTelegraph(World, SpawnTransform, Config.SpawnDelay) : nullptr;

	const TWeakObjectPtr<AActor> WeakOwner = PolicyContext.OwnerActor;
	const TWeakObjectPtr<AActor> WeakAttachParent = AttachParent;
	const TSubclassOf<AActor> ActorClass = Config.ActorClass;
	const bool bAttachToActor = Config.bAttachToActor;

	// 히트 기록은 강하게 쥔다. 어빌리티가 끝난 뒤에 나와도 배율을 이어받는다.
	FTimerHandle DelayHandle;
	World->GetTimerManager().SetTimer(DelayHandle, FTimerDelegate::CreateWeakLambda(World,
		[World, HitContext, WeakOwner, WeakAttachParent, ActorClass, bAttachToActor, SpawnTransform, WeakTelegraph]()
		{
			if ( AYSTelegraphActor* Telegraph = WeakTelegraph.Get() )
			{
				Telegraph->Destroy();
			}

			if ( WeakOwner.IsValid() == false )
				return;

			SpawnAtTransform(World, ActorClass, bAttachToActor, SpawnTransform, WeakOwner.Get(), WeakAttachParent.Get(), HitContext);
		}), Config.SpawnDelay, false);

	if ( bCancelOnEnd )
	{
		HitContext->OnOwnerEndedEvent.AddWeakLambda(World, [World, DelayHandle, WeakTelegraph]() mutable
		{
			World->GetTimerManager().ClearTimer(DelayHandle);

			if ( AYSTelegraphActor* Telegraph = WeakTelegraph.Get() )
			{
				Telegraph->Destroy();
			}
		});
	}

	return nullptr;
}

FTransform UYSBlueprintFunctionLibrary::CalculateSpawnTransform(UObject* WorldContext,
	const FYSSpawnActorConfig& Config, const FYSTransformPolicyContext& PolicyContext)
{
	const FTransform PolicyTransform = Config.TransformPolicy.GetFinalTransform(PolicyContext);

	FVector Position = PolicyTransform.GetLocation();
	const FRotator Rotation = PolicyTransform.Rotator();

	if (Config.bStickGround)
	{
		if (UNavigationSystemV1* NavSystem = UNavigationSystemV1::GetCurrent(WorldContext))
		{
			FNavLocation NavLocation;
			if (NavSystem->ProjectPointToNavigation(Position, NavLocation, FVector(0.f, 0.f, GetDefault<UYSDeveloperSettings>()->NavProjectionMaxHeight)))
			{
				Position = NavLocation.Location;
			}
		}
	}

	return FTransform(Rotation, Position);
}

void UYSBlueprintFunctionLibrary::ApplyHitEffects(AActor* Source, AActor* Instigator, AActor* Target,
	const FName& SkillID, const FHitResult& HitResult)
{
	const FYSDamageInfo* DamageInfo = UYSDeveloperSettings::GetDamageInfo(SkillID);

	if ( DamageInfo == nullptr || DamageInfo->DamageEffects.Num() == 0 )
		return;
	
	FYSDamageEffectContext Context;
	
	Context.Source = Source;
	Context.Instigator = Instigator;
	Context.Target = Target;
	Context.HitResult = HitResult;
	
	for ( const FInstancedStruct& Instanced : DamageInfo->DamageEffects )
	{
		const FYSDamageEffectBase* Effect = Instanced.GetPtr<FYSDamageEffectBase>();

		if ( Effect != nullptr && Effect->CanApply(Context) )
		{
			Effect->Apply(Context);
		}
	}
}

AActor* UYSBlueprintFunctionLibrary::ResolveDamageInstigator(AActor* Actor)
{
	if ( const IYSDamageProxy* Proxy = Cast<IYSDamageProxy>(Actor) )
	{
		return Proxy->GetDamageInstigator();
	}

	return Actor;
}

bool UYSBlueprintFunctionLibrary::ApplyHitToTarget(AActor* Source, AActor* DamageInstigator, AActor* Target,
	const FName& DamageRow, float EffectScale, const FHitResult& HitResult, const FRotator& EffectRotation,
	TSet<TWeakObjectPtr<AActor>>* JustAvoidGranted)
{
	if ( AdmitHit(DamageInstigator, Target, JustAvoidGranted) == false )
		return false;

	const FName ResolvedRow = ResolveDamageRow(Target, DamageRow);

	SendHitEventToTarget(DamageInstigator, Target, ResolvedRow, EffectScale);
	SpawnEffects(DamageInstigator, ResolvedRow, HitResult.ImpactPoint, EffectRotation);
	ApplyHitEffects(Source, DamageInstigator, Target, ResolvedRow, HitResult);

	return true;
}

