// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/YSBlueprintFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "YSBattleActor.h"
#include "YSDeveloperSettings.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "AttackableActor/YSAttackableBase.h"
#include "AttackableActor/DamageEffect/YSDamageEffect.h"
#include "Character/YSCharacterBase.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "Character/Components/YSCameraManageComponent.h"
#include "Character/Components/YSTargetingComponent.h"
#include "General/YSDefine.h"
#include "General/YSGameplayTag.h"
#include "Interface/YSDamageProxy.h"
#include "Subsystem/YSObjectPoolingSubsystem.h"

namespace
{
	constexpr float RelativeEvaluationBonus = 1.15f;

	float GetRelativeEvaluationMultiplier(const UYSCharacterAttributeSetBase* Owner, const UYSCharacterAttributeSetBase* Target)
	{
		const UAbilitySystemComponent* OwnerASC = Owner->GetOwningAbilitySystemComponent();
		if (IsValid(OwnerASC) == false || OwnerASC->HasMatchingGameplayTag(YSTags::Passive_RelativeEvaluation) == false)
			return 1.f;

		// HP 비율 비교를 교차곱으로 해서 MaxHp 0 나눗셈을 피한다
		const bool bTargetHealthier = Owner->GetHp() * Target->GetMaxHp() < Target->GetHp() * Owner->GetMaxHp();
		return bTargetHealthier ? RelativeEvaluationBonus : 1.f;
	}
}

float UYSBlueprintFunctionLibrary::GetFinalDamage(const UYSCharacterAttributeSetBase* Owner,
                                                  const UYSCharacterAttributeSetBase* Target, const FName& SkillID)
{
	const FYSDamageInfo* DamageInfo = UYSDeveloperSettings::GetDamageInfo(SkillID);

	if (IsValid(Owner) == false || IsValid(Target) == false )
		return 0.f;

	// 데미지 정보가 없으면 의미 X
	if ( DamageInfo == nullptr )
		return 0.f;

	float BaselineDamage = DamageInfo->DamageType == EYSDamageType::Melee ? Owner->GetMeleeAttackDmg() : Owner->GetRangedAttackDmg();

	// (공격 데미지 * 공격 계수) * (1 - DefenceRate) * 받는 피해 배율 * 상대평가 + 고정 데미지. 고정 데미지는 배율을 받지 않는다.
	float ScaledDamage = ( BaselineDamage * DamageInfo->DamageMultiplier ) * ( 1 - Target->GetDefenseRate()) * Target->GetIncomingDamageMultiplier();
	ScaledDamage *= GetRelativeEvaluationMultiplier(Owner, Target);

	return ScaledDamage + DamageInfo->AdditiveTrueDamage;
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
	
	float FinalDamage = GetFinalDamage(ASC->GetSet<UYSCharacterAttributeSetBase>(), TargetASC->GetSet<UYSCharacterAttributeSetBase>(), SkillID);
	TargetASC->SetNumericAttributeBase(UYSCharacterAttributeSetBase::GetIncomingDamageAttribute(), FinalDamage);
	
	FGameplayEventData EventData;
	EventData.Instigator = Instigator;
	EventData.Target = Target;
	EventData.EventMagnitude = FinalDamage;

	TargetASC->HandleGameplayEvent(DamageInfo->HitTag, &EventData);
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

AActor* UYSBlueprintFunctionLibrary::SpawnByConfig(UObject* WorldContext, const FYSSpawnActorConfig& Config,
	const FYSTransformPolicyContext& PolicyContext, AActor* AttachParent, const TSharedPtr<FYSAbilityHitContext>& HitContext)
{
	if (IsValid(WorldContext) == false)
		return nullptr;

	UWorld* World = WorldContext->GetWorld();
	if (IsValid(World) == false || Config.ActorClass == nullptr )
		return nullptr;

	const FTransform SpawnTransform = CalculateSpawnTransform(WorldContext, Config, PolicyContext);

	AActor* PooledActor = AcquirePooledActor(World, Config.ActorClass, SpawnTransform);
	if (IsValid(PooledActor) == false)
		return nullptr;

	AActor* OwnerActor = PolicyContext.OwnerActor;
	AActor* Instigator = OwnerActor;
	if ( const IYSDamageProxy* Proxy = Cast<IYSDamageProxy>(OwnerActor) )
	{
		Instigator = Proxy->GetDamageInstigator();
	}
	
	if (Config.bAttachToActor && IsValid(AttachParent))
	{
		PooledActor->AttachToActor(AttachParent, FAttachmentTransformRules::KeepWorldTransform);
	}
	
	if ( IYSSpawnInitializable* Initializable = Cast<IYSSpawnInitializable>(PooledActor) )
	{
		if ( Initializable->OnSpawnInitialize(OwnerActor, Instigator, HitContext) == false )
		{
			if ( UYSObjectPoolingSubsystem* ObjectPooling = UYSObjectPoolingSubsystem::Get(World) )
			{
				ObjectPooling->ReturnPooledActor(PooledActor);
			}
			return nullptr;
		}
		
		Initializable->SetPoolActive(true);
	}
	
	return PooledActor;
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
			if (NavSystem->ProjectPointToNavigation(Position, NavLocation, FVector(0.f, 0.f, YS_PROJECTION_MAX_DISTANCE)))
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

	// 데미지 정보가 없으면 의미 X
	if ( DamageInfo == nullptr || DamageInfo->DamageEffects.Num() == 0 )
		return;
	
	FYSDamageEffectContext Context;
	
	Context.Source = Source;
	Context.Instigator = Instigator;
	Context.Target = Target;
	Context.HitResult = HitResult;
	
	for ( const FInstancedStruct& Instanced : DamageInfo->DamageEffects )
	{
		if ( const FYSDamageEffectBase* Effect = Instanced.GetPtr<FYSDamageEffectBase>() )
		{
			Effect->Apply(Context);
		}
	}
}

