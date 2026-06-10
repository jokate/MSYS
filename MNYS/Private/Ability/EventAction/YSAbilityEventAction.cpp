// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/EventAction/YSAbilityEventAction.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemInterface.h"
#include "Abilities/Tasks/AbilityTask_ApplyRootMotionMoveToForce.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Ability/Payload/YSAbilityTriggerPayload.h"
#include "Ability/Task/YSAT_Trace.h"
#include "AttackableActor/YSAttackableActor.h"
#include "Character/YSCharacterBase.h"
#include "Character/YSPlayerController.h"
#include "Character/Components/YSLockOnComponent.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"
#include "Library/YSBlueprintFunctionLibrary.h"

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

void UYSAbilityEventAction_Destroy::DestroyActor()
{
	if ( DestroyTarget.IsValid() == false )
		return;
	
	DestroyTarget->Destroy();
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

bool UYSAbilityEventAction_TransitionState::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	if ( IsValid(OwningAbility) == false )
		return false;

	AActor* AvatarActor = OwningAbility->GetAvatarActorFromActorInfo();
	UYSInputStateMachineComponent* StateMachineComponent = UYSInputStateMachineComponent::Get(AvatarActor);
	
	if ( IsValid(StateMachineComponent) == false )
		return false;
	
	StateMachineComponent->AddStateStack(NextState);
	
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
	
	FGameplayEffectContextHandle EffectContextHandle;
	EffectContextHandle.SetAbility(OwningAbility);
	FGameplayEffectSpec Spec(GameplayEffect.GetDefaultObject(), EffectContextHandle);	
	
	if ( bRemoveWhenFinished )
	{
		OwningAbility->AddRuntimeEffectSpecHandle(OwnerASC->ApplyGameplayEffectSpecToSelf(Spec));
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
	
	UAbilitySystemComponent* TargetASC = ASI->GetAbilitySystemComponent();
	UAbilitySystemComponent* OwnerASC = OwningAbility->GetAbilitySystemComponentFromActorInfo();
	if ( IsValid(TargetASC) == false || IsValid(OwnerASC) == false )
		return false;
	
	
	FGameplayEffectContextHandle EffectContextHandle;
	EffectContextHandle.SetAbility(OwningAbility);
	FGameplayEffectSpec Spec(GameplayEffect.GetDefaultObject(), EffectContextHandle);	
	
	if (bRemoveWhenFinished)
	{
		OwningAbility->AddRuntimeEffectSpecHandle(TargetASC->ApplyGameplayEffectSpecToTarget(Spec, OwnerASC));	
	}
	
	return true;
}

bool UYSAbilityEventAction_ApplyVelocity::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	const UYSAbilityTriggerPayload_Velocity* VelocityData = UYSAbilityTriggerPayload::GetPayload<UYSAbilityTriggerPayload_Velocity>(&EventData);;

	if ( IsValid(VelocityData) == false)
		return false;
	
	AActor* OwnerActor = OwningAbility->GetOwningActorFromActorInfo();
	
	FRotator DirectionRotation = UYSBlueprintFunctionLibrary::GetAbilityEventRotation(VelocityData->VelocityDirectionPolicy, OwningAbility, NAME_None);
	
	FVector DirectionVector = DirectionRotation.Vector();
	const FVector TargetLocation = OwnerActor->GetActorLocation() 
		+ DirectionVector
		* ( VelocityData->Velocity * VelocityData->Duration );
	
	// 뒤로 쭉 밀려서 들어가야 하는 경우라면?
	if ( VelocityData->bRotateActorToDirection )
	{
		OwnerActor->SetActorRotation(DirectionVector.Rotation());	
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

	UWorld* World = OwningAbility->GetWorld();
	if (IsValid(World) == false)
		return false;

	for (const FYSSpawnActorConfig& SpawnActorConfig : SpawnActorPayload->SpawnActorConfigs)
	{
		FTransform SpawnTransform = CalculateTransform(OwningAbility, SpawnActorConfig);
		AActor* SpawnedActor      = World->SpawnActorDeferred<AActor>(SpawnActorConfig.ActorClass, SpawnTransform);

		if (IsValid(SpawnedActor) == false)
			continue;

		AYSAttackableActor* AttackableActor = Cast<AYSAttackableActor>(SpawnedActor);
		if (IsValid(AttackableActor))
		{
			AttackableActor->AllocateInstigator(OwningActor);
		}

		if (SpawnActorConfig.bAttachToActor)
		{
			SpawnedActor->AttachToActor(OwningActor, FAttachmentTransformRules::KeepWorldTransform);
		}

		SpawnedActor->FinishSpawning(SpawnTransform);
	}

	return true;
}

FTransform UYSAbilityEventAction_SpawnActor::CalculateTransform(UYSGameplayAbility* OwningAbility,
	const FYSSpawnActorConfig& SpawnConfig)
{
	const FVector Position = UYSBlueprintFunctionLibrary::GetAbilityEventPosition(SpawnConfig.PositionPolicy, OwningAbility, SpawnConfig.SpawnSocket, SpawnConfig.RelativeOffset);
	const FRotator  Rotation = UYSBlueprintFunctionLibrary::GetAbilityEventRotation(SpawnConfig.RotationPolicy, OwningAbility, SpawnConfig.RotationSocket);

	return FTransform(Rotation, Position);
}

bool UYSAbilityEventAction_CheckContextTag::Execute_Implementation(UYSGameplayAbility* OwningAbility,
	const FGameplayEventData& EventData)
{
	if ( IsValid(OwningAbility) == false )
		return false;

	UYSAbilityPlaybackBase* PlaybackBase = const_cast<UYSAbilityPlaybackBase*>(OwningAbility->GetCurrentPlayback());
	
	if (IsValid(PlaybackBase) == false)
		return false;
	
	PlaybackBase->DispatchNext(EYSPlaybackEvent::OnCheckContextTag, true);
	
	return true;
}

