// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/EventAction/YSAbilityEventAction.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Ability/Payload/YSAbilityTriggerPayload.h"
#include "Ability/Task/YSAT_RunEQSQuery.h"
#include "Ability/Task/YSAT_Trace.h"
#include "AttackableActor/YSDamagableActor.h"
#include "AttackableActor/YSSaveEcho.h"
#include "Character/YSAnimInstance.h"
#include "Character/YSCharacterBase.h"
#include "Character/YSPlayerController.h"
#include "Character/Components/YSCharacterMovementComponent.h"
#include "Character/Components/YSCameraManageComponent.h"
#include "Character/Components/YSSaveComponent.h"
#include "General/YSDefine.h"
#include "General/YSGameplayTag.h"
#include "Library/YSBlueprintFunctionLibrary.h"
#include "Subsystem/YSObjectPoolingSubsystem.h"
#include "YSBattleActor.h"
#include "Components/PrimitiveComponent.h"
#include "Engine/HitResult.h"

bool UYSAbilityEventAction_StartTrace::Execute_Implementation(UYSGameplayAbility* OwningAbility,
                                                              const FGameplayEventData& EventData)
{
	const UYSAbilityTriggerPayload_Trace* TraceData = UYSAbilityTriggerPayload::GetPayload<UYSAbilityTriggerPayload_Trace>(&EventData);

	if ( IsValid(TraceData) == false)
		return false;
	
	UYSAT_Trace* TraceTask = UYSAT_Trace::CreateTask(OwningAbility, TraceData->TraceConfig);
	if ( IsValid(TraceTask) )
	{
		OwningAbility->SetTraceTask(TraceTask);
		TraceTask->ReadyForActivation();
	}
	
	return true;
}

bool UYSAbilityEventAction_StopTrace::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	if ( IsValid(OwningAbility) == false )
		return false;

	UYSAT_Trace* TraceTask = OwningAbility->GetTraceTask();

	if ( IsValid(TraceTask) == false )
		return false;
	
	TraceTask->EndTask();
	
	return true;
}

bool UYSAbilityEventAction_HitContextTarget::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	if ( IsValid(OwningAbility) == false )
		return false;

	const UYSAbilityTriggerPayload_DirectHit* Payload = UYSAbilityTriggerPayload::GetPayload<UYSAbilityTriggerPayload_DirectHit>(&EventData);

	if ( IsValid(Payload) == false )
		return false;

	const TSharedPtr<FYSPlaybackContext>& Context = OwningAbility->GetAbilityPlaybackContext();

	if ( Context.IsValid() == false )
		return false;

	AActor* Instigator = Context->Instigator;
	AActor* Target = Context->Target;

	if ( IsValid(Instigator) == false || IsValid(Target) == false )
		return false;

	// 분신은 팀 · 속성 세트가 없어 AdmitHit 이 막으므로 Master 로 푼다.
	AActor* DamageInstigator = UYSBlueprintFunctionLibrary::ResolveDamageInstigator(Instigator);

	// 시퀀스가 두 액터의 위치를 쥐고 있으므로 스윕 대신 타겟 위치를 히트 지점으로 쓴다.
	const FVector HitLocation = Target->GetActorLocation();
	const FVector HitDirection = (HitLocation - Instigator->GetActorLocation()).GetSafeNormal2D();
	const FHitResult HitResult(Target, Cast<UPrimitiveComponent>(Target->GetRootComponent()), HitLocation, -HitDirection);

	// 걸러지면 히트 컨텍스트 기록 · OnHit 도 건너뛴다.
	if ( UYSBlueprintFunctionLibrary::ApplyHitToTarget(Instigator, DamageInstigator, Target, Payload->DamageRow,
		OwningAbility->GetEffectScale(), HitResult, HitDirection.ToOrientationRotator()) == false )
		return false;

	TArray<AActor*> HitActors;
	HitActors.Add(Target);
	UYSBlueprintFunctionLibrary::PlayHitImpact(Instigator, HitActors, Payload->DamageRow);

	TArray<FHitResult> HitResults;
	HitResults.Add(HitResult);

	if ( FYSAbilityHitContext* HitContext = OwningAbility->GetHitContext().Get() )
	{
		HitContext->AddHitActor(Target);
		HitContext->UpdateHitResult(this, HitResult);
	}

	if ( UYSAbilityPlaybackBase* Playback = OwningAbility->GetCurrentPlayback() )
	{
		Playback->OnHit(HitResults);
	}

	return true;
}

void UYSAbilityEventAction_Destroy::DestroyActor()
{
	if ( DestroyTarget.IsValid() == false )
		return;
	
	UYSObjectPoolingSubsystem::ReturnToPool(DestroyTarget.Get());
}

bool UYSAbilityEventAction_Destroy::Execute_Implementation(UYSGameplayAbility* OwningAbility,
                                                           const FGameplayEventData& EventData)
{	
	if ( IsValid(OwningAbility) == false )
		return false;

	UWorld* World = OwningAbility->GetWorld();

	if ( IsValid(World) == false )
		return false;
	
	DestroyTarget = OwningAbility->GetAvatarActorFromActorInfo();
	
	World->GetTimerManager().SetTimer(DeathTimerHandle, this, &ThisClass::DestroyActor, DestroyTime, false);
	
	return true;
}

bool UYSAbilityEventAction_GameplayCue::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	if ( IsValid(OwningAbility) == false )
		return false;
	
	UAbilitySystemComponent* ASC = OwningAbility->GetAbilitySystemComponentFromActorInfo();
	
	if ( IsValid(ASC) == false )
		return false;
	
	for ( const FGameplayTag& CueTag : GameplayCueTag )
	{
		ASC->AddGameplayCue(CueTag);

		// 푸는 액션이 따로 없다. 건 만큼만 어빌리티가 끝날 때 푼다.
		OwningAbility->AddEndCleanup([WeakASC = TWeakObjectPtr<UAbilitySystemComponent>(ASC), CueTag]()
		{
			if ( UAbilitySystemComponent* CueASC = WeakASC.Get() )
			{
				CueASC->RemoveGameplayCue(CueTag);
			}
		});
	}
	
	return true;
}

bool UYSAbilityEventAction_GameplayEffect::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	if ( IsValid(OwningAbility) == false )
		return false;

	return bIsFromInstigator ? Execute_GameplayEffectFromInstigator(OwningAbility, EventData) 
		: Execute_GameplayEffectToSelf(OwningAbility, EventData);
}

bool UYSAbilityEventAction_GameplayEffect::Execute_GameplayEffectToSelf(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	UAbilitySystemComponent* OwnerASC = OwningAbility->GetAbilitySystemComponentFromActorInfo();
	if ( IsValid(OwnerASC) == false )
		return false;
	
	FGameplayEffectContextHandle EffectContextHandle = OwnerASC->MakeEffectContext();
	EffectContextHandle.SetAbility(OwningAbility);
	EffectContextHandle.AddInstigator(OwningAbility->GetAvatarActorFromActorInfo(), OwningAbility->GetOwningActorFromActorInfo());
	FGameplayEffectSpec Spec(GameplayEffect.GetDefaultObject(), EffectContextHandle);	
	
	const FActiveGameplayEffectHandle Handle = OwnerASC->ApplyGameplayEffectSpecToSelf(Spec);
	if ( bRemoveWhenFinished )
	{
		OwningAbility->AddRuntimeEffectSpecHandle(Handle);
	}
	return true;
}

bool UYSAbilityEventAction_GameplayEffect::Execute_GameplayEffectFromInstigator(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	if ( IsValid(EventData.Instigator) == false )
		return false;
	
	const IAbilitySystemInterface* ASI = Cast<const IAbilitySystemInterface>(EventData.Instigator);
	
	if ( ASI == nullptr )
		return false;
	
	UAbilitySystemComponent* InstigatorASC = ASI->GetAbilitySystemComponent();
	UAbilitySystemComponent* OwnerASC = OwningAbility->GetAbilitySystemComponentFromActorInfo();
	if ( IsValid(InstigatorASC) == false || IsValid(OwnerASC) == false )
		return false;

	// 받는 쪽은 나, 거는 쪽(원인 · 수치 출처)은 가해자다.
	FGameplayEffectContextHandle EffectContextHandle = InstigatorASC->MakeEffectContext();
	FGameplayEffectSpec Spec(GameplayEffect.GetDefaultObject(), EffectContextHandle);

	const FActiveGameplayEffectHandle Handle = InstigatorASC->ApplyGameplayEffectSpecToTarget(Spec, OwnerASC);
	
	if (bRemoveWhenFinished)
	{
		OwningAbility->AddRuntimeEffectSpecHandle(Handle);	
	}
	
	return true;
}

void UYSAbilityEventAction_ApplyVelocity::OnTimedOut()
{
	// 비행 모드로 끝나면 어빌리티가 끝날 때까지 떠 있어야 한다.
	if ( VelocityData->bSetNewMovementMode && VelocityData->MoveMode != MOVE_Flying )
	{
		if ( UYSCharacterMovementComponent* CharacterMovement = UYSCharacterMovementComponent::Get(OwningActor) )
		{
			CharacterMovement->ResumeGravity(this);
		}
	}
}

bool UYSAbilityEventAction_ApplyVelocity::Execute_Implementation(UYSGameplayAbility* OwningAbility,
                                                                 const FGameplayEventData& EventData)
{
	VelocityData = UYSAbilityTriggerPayload::GetPayload<UYSAbilityTriggerPayload_Velocity>(&EventData);;

	if ( IsValid(VelocityData) == false)
		return false;
	
	OwningActor = OwningAbility->GetOwningActorFromActorInfo();
	
	AActor* PlaybackTarget = nullptr;
	if (const UYSAbilityPlaybackBase* Playback = OwningAbility->GetCurrentPlayback())
	{
		PlaybackTarget = Playback->GetCurrentPlaybackTarget();
	}

	const FYSRotationPolicyBase* DirectionPolicy = VelocityData->VelocityDirectionPolicy.GetPtr<FYSRotationPolicyBase>();
	const FRotator DirectionRotation = DirectionPolicy
		? DirectionPolicy->GetRotation(FYSTransformPolicyContext(OwningActor, PlaybackTarget))
		: OwningActor->GetActorRotation();
	FVector DirectionVector = DirectionRotation.Vector();
	const FVector TargetLocation = OwningActor->GetActorLocation() 
		+ DirectionVector
		* ( VelocityData->Velocity * VelocityData->Duration );
	
	if ( VelocityData->bRotateActorToDirection )
	{
		if ( UYSCharacterMovementComponent* Movement = UYSCharacterMovementComponent::Get(OwningActor) )
		{
			Movement->RequestFaceYaw(DirectionVector.Rotation().Yaw, EYSFaceYawSource::AbilityMotion);
		}
		else
		{
			OwningActor->SetActorRotation(DirectionVector.Rotation());
		}
	}
	
	if (VelocityData->bSetNewMovementMode)
	{
		if ( UYSCharacterMovementComponent* CharacterMovement = UYSCharacterMovementComponent::Get(OwningActor) )
		{
			CharacterMovement->SuspendGravity(this);

			// 시간 전에 어빌리티가 끝나면 태스크가 OnTimedOut 없이 사라지므로 종료 정리에서 되돌린다.
			OwningAbility->AddEndCleanup([WeakMovement = TWeakObjectPtr<UYSCharacterMovementComponent>(CharacterMovement), WeakAction = TWeakObjectPtr<UYSAbilityEventAction_ApplyVelocity>(this)]()
			{
				if ( UYSCharacterMovementComponent* Movement = WeakMovement.Get() )
				{
					Movement->ResumeGravity(WeakAction.Get());
				}
			});
		}
	}
	
	UAbilityTask_ApplyRootMotionMoveToForce* ApplyVelocityTask = 
		UAbilityTask_ApplyRootMotionMoveToForce::ApplyRootMotionMoveToForce(OwningAbility, 
			TEXT("AppyVelocity"), TargetLocation, VelocityData->Duration,
			VelocityData->bSetNewMovementMode,
			VelocityData->MoveMode,
			true,
			nullptr,
			VelocityData->FinishVelocityMode, 
			VelocityData->FinishSetVelocity, 
			VelocityData->FinishClampVelocity);
	
	if (IsValid(ApplyVelocityTask) == false )
	{
		return false;
	}
	
	ApplyVelocityTask->OnTimedOut.AddDynamic(this, &UYSAbilityEventAction_ApplyVelocity::OnTimedOut);
	ApplyVelocityTask->OnTimedOutAndDestinationReached.AddDynamic(this, &UYSAbilityEventAction_ApplyVelocity::OnTimedOut);
	ApplyVelocityTask->ReadyForActivation();
	
	return true;
}


bool UYSAbilityEventAction_SpawnActor::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	const UYSAbilityTriggerPayload_SpawnActor* SpawnActorPayload = UYSAbilityTriggerPayload::GetPayload<UYSAbilityTriggerPayload_SpawnActor>(&EventData);

	if (IsValid(SpawnActorPayload) == false)
		return false;

	AActor* OwningActor = OwningAbility->GetOwningActorFromActorInfo();
	if (IsValid(OwningActor) == false)
		return false;

	AActor* PlaybackTarget = nullptr;
	if (const UYSAbilityPlaybackBase* Playback = OwningAbility->GetCurrentPlayback())
	{
		PlaybackTarget = Playback->GetCurrentPlaybackTarget();
	}

	const FYSTransformPolicyContext PolicyContext(OwningActor, PlaybackTarget);

	for (const FYSSpawnActorConfig& SpawnActorConfig : SpawnActorPayload->SpawnActorConfigs)
	{
		UYSBlueprintFunctionLibrary::SpawnByConfig(OwningAbility, SpawnActorConfig, PolicyContext, OwningActor, OwningAbility->GetHitContext());
	}

	return true;
}

bool UYSAbilityEventAction_CheckContextTag::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	if ( IsValid(OwningAbility) == false )
		return false;

	UYSAbilityPlaybackBase* PlaybackBase = const_cast<UYSAbilityPlaybackBase*>(OwningAbility->GetCurrentPlayback());
	
	if (IsValid(PlaybackBase) == false)
		return false;
	
	PlaybackBase->HandleContextTagChanged();
	
	return true;
}

bool UYSAbilityEventAction_RunEQS::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	const UYSAbilityTriggerPayload_EQS* AbilityTrigger = UYSAbilityTriggerPayload::GetPayload<UYSAbilityTriggerPayload_EQS>(&EventData);
	
	if ( IsValid(AbilityTrigger) == false )
	{
		return false;
	}
	
	UYSAT_RunEQSQuery* EQSTask = UYSAT_RunEQSQuery::CreateTask(OwningAbility, AbilityTrigger->TargetToRun);
	
	if ( IsValid(EQSTask) == false )
	{
		return false;
	}
	
	EQSTask->OnSucceeded.BindUObject(this, &UYSAbilityEventAction_RunEQS::OnEQSQueryFinished);
	EQSTask->ReadyForActivation();
	return true;
}

void UYSAbilityEventAction_RunEQS::OnEQSQueryFinished_Implementation(UYSGameplayAbility* OwningAbility,
	FVector Location)
{
	UE_LOG(LogTemp, Log, TEXT("EQS Query Completed"));
}

bool UYSAbilityEventAction_AimStart::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	const UYSAbilityTriggerPayload_AimTarget* AimTarget = UYSAbilityTriggerPayload::GetPayload<UYSAbilityTriggerPayload_AimTarget>(&EventData);
	
	if ( IsValid(AimTarget) == false || IsValid(OwningAbility) == false )
	{
		return false;
	}
	
	AActor* AvatarActor = OwningAbility->GetAvatarActorFromActorInfo();
	
	if ( IsValid(AvatarActor) == false )
	{
		return false;
	}
	
	UYSTargetingComponent* TargetingComponent = UYSTargetingComponent::Get(AvatarActor);
	
	if ( IsValid(TargetingComponent) == false ) 
	{
		return false;
	}
	
	TargetingComponent->BeginTargeting(OwningAbility, AimTarget->TargetingSpec);
	return true;
}

bool UYSAbilityEventAction_AimStop::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	if ( IsValid(OwningAbility) == false )
	{
		return false;
	}
	
	AActor* AvatarActor = OwningAbility->GetAvatarActorFromActorInfo();
	
	if ( IsValid(AvatarActor) == false )
	{
		return false;
	}
	
	UYSTargetingComponent* TargetingComponent = UYSTargetingComponent::Get(AvatarActor);
	
	if ( IsValid(TargetingComponent) == false ) 
	{
		return false;
	}
	
	TargetingComponent->EndTargeting(OwningAbility);
	return true;
}

bool UYSAbilityEventAction_ConsumeResource::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	if ( IsValid(OwningAbility) == false )
		return false;
	
	UAbilitySystemComponent* ASC = OwningAbility->GetAbilitySystemComponentFromActorInfo();
	
	if ( IsValid(ASC) == false )
		return false;
	
	
	float CurrentValue = ASC->GetNumericAttribute(Attribute);
	if (CurrentValue < Amount)
	{
		return false;
	}
	
	ASC->ApplyModToAttribute(Attribute, EGameplayModOp::Additive, -Amount);
	return true;
}

bool UYSAbilityEventAction_SaveAbilityRecord::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	if ( IsValid(OwningAbility) == false )
		return false;
	
	AActor* AvatarActor = OwningAbility->GetAvatarActorFromActorInfo();
	
	if ( IsValid(AvatarActor) == false )
	{
		return false;
	}
	
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(AvatarActor);
	if ( IsValid(ASC) == false )
	{
		return false;
	}

	// 분신이 재현 중 다시 Record 를 쏘면 분신이 분신을 낳는다. 여기서 끊는다.
	if ( ASC->HasMatchingGameplayTag(YSTags::State_Echo) )
	{
		return false;
	}

	UYSSaveComponent* SaveComponent = UYSSaveComponent::Get(AvatarActor);
	if ( IsValid(SaveComponent) == false )
	{
		return false;
	}
	
	
	if ( ASC->HasMatchingGameplayTag(YSTags::State_Save_TripleEcho) )
	{
		const FYSSavedTechnique Technique(OwningAbility->GetClass(), OwningAbility->GetCurrentPlaybackIndex(), 1.0f);
		SaveComponent->ExecuteTripleEcho(Technique, OwningAbility->GetHitContext());
		return true;
	}
	
	SaveComponent->MarkSavable(OwningAbility->GetClass(), OwningAbility->GetCurrentPlaybackIndex(), 1.0f);
	return true;
}

bool UYSAbilityEventAction_LandingEvent::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	if ( IsValid(OwningAbility) == false )
		return false;
	
	AActor* AvatarActor = OwningAbility->GetAvatarActorFromActorInfo();
	
	if ( IsValid(AvatarActor) == false )
	{
		return false;
	}
	
	Execute_JumpToLanding(AvatarActor);
	
	return true;
}

void UYSAbilityEventAction_LandingEvent::Execute_JumpToLanding(AActor* OwnerActor)
{
	if ( IsValid(OwnerActor) == false )
	{
		return;
	}
	
	UYSAnimInstance* AnimInstance = UYSAnimInstance::GetAnimInstance(OwnerActor);
	if ( IsValid(AnimInstance) == false )
	{
		return;
	}
	
	AnimInstance->Montage_JumpToSection(MontageSectionName);
}
