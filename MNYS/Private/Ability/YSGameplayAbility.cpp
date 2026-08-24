// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/YSGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "YSAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Ability/EventAction/YSAbilityEventAction.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Ability/AbilityComponent/YSPlaybackGraphAsset.h"
#include "Character/YSCharacterPlayer.h"
#include "Character/Components/YSCharacterMovementComponent.h"
#include "Character/Components/YSTargetingComponent.h"
#include "General/YSGameplayTag.h"
#include "Library/YSBlueprintFunctionLibrary.h"
#include "Subsystem/YSWorldTagSubsystem.h"

UYSGameplayAbility::UYSGameplayAbility()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
}

void UYSGameplayAbility::OnGameplayTagChanged(const FGameplayTag& Tag, bool bInIsActive)
{
	if ( Tag == YSTags::AcceptAbilityInput )
	{
		SetInputAcceptable(bInIsActive);
		
		UYSAbilityPlaybackBase* PlaybackBase = CurrentPlayback.Get();
	
		if (IsValid(PlaybackBase) == false)
			return;
	
		// 입력에 대한 심사가 끝났다면 인풋에 대한 컨텍스트 처리를 담당해야 한다.
		if (!bInIsActive)
		{
			PlaybackBase->DispatchNext(EYSPlaybackEvent::OnInput, false);	
		}
	}
}

void UYSGameplayAbility::ActivePlayback(int32 Index)
{
	const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& PlaybackArray = GetPlaybackArray();

	if ( PlaybackArray.IsValidIndex(Index) == false )
		return;

	UYSAbilityPlaybackBase* Playback = PlaybackArray[Index];

	if ( IsValid(CurrentPlayback.Get()))
	{
		CurrentPlayback->EndPlay();	
	}
	
	if ( IsValid(Playback) == false )
		return;

	CurrentPlayback = Playback;
	CurrentPlaybackIndex = Index;

	// 시전이 확정되는 노드에서 커밋한다.
	// NeedReady 어빌리티는 활성 시점에 커밋을 미뤄뒀으므로 여기가 쿨다운이 도는 지점이다.
	// 조준 단계에서 취소되면 이 노드에 도달하지 않아 쿨다운이 소모되지 않는다.
	if ( Playback->bCommitOnEnter && bHasCommitted == false )
	{
		bHasCommitted = true;
		CommitAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo);
	}

	CurrentPlayback->Play(PlaybackContext);
}

bool UYSGameplayAbility::CheckCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, FGameplayTagContainer* OptionalRelevantTags) const
{
	UYSAbilitySystemComponent* YSASC = Cast<UYSAbilitySystemComponent>(ActorInfo->AbilitySystemComponent);
	
	const FYSSkillInfo* SkillInfo = GetSkillInfo();
	
	if ( IsValid(YSASC) == false || SkillInfo == nullptr )
	{
		return true;
	}

	if ( SkillInfo->bHasCooldown == false )
	{
		return true;
	}
	
	return !YSASC->IsOnCooldown(GetClass());
}

void UYSGameplayAbility::ApplyCooldown(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo) const
{
	UYSAbilitySystemComponent* YSASC = Cast<UYSAbilitySystemComponent>(ActorInfo->AbilitySystemComponent);
	const FYSSkillInfo* SkillInfo = GetSkillInfo();
	
	if ( IsValid(YSASC) == false || SkillInfo == nullptr )
	{
		return;
	}

	if ( SkillInfo->bHasCooldown == false )
	{
		return;
	}
	
	YSASC->StartCoolDown(GetClass(), GetWorld()->GetTimeSeconds() + SkillInfo->Cooldown);
}

void UYSGameplayAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                         const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                         const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	UYSAbilitySystemComponent* YSASC = Cast<UYSAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());

	if ( IsValid(YSASC) )
	{
		YSASC->OnGameplayTagStateChanged.AddDynamic(this, &UYSGameplayAbility::OnGameplayTagChanged);
	}
	
	AActor* OwnerActor = GetOwningActorFromActorInfo();

	if ( IsValid(OwnerActor) )
	{
		AYSCharacterPlayer* Character = Cast<AYSCharacterPlayer>(OwnerActor);

		if ( IsValid(Character) )
		{
			Character->AddStateToStateMachine(ChangeInputStateType);
		}
	}
	
	if ( bBlockMovementDuringAbility )
	{
		UYSCharacterMovementComponent* MovementComponent = UYSCharacterMovementComponent::Get(OwnerActor);
		
		if ( IsValid(MovementComponent) )
		{
			MovementComponent->SetMovementBlocked(true);
		}
	}

	if ( bFreezeInAirDuringAbility )
	{
		UYSCharacterMovementComponent* MovementComponent = UYSCharacterMovementComponent::Get(OwnerActor);

		// 지상에서 켜면 의미가 없고, 복구 대상만 늘어난다.
		if ( IsValid(MovementComponent) && MovementComponent->IsFalling() )
		{
			CachedGravityScale = MovementComponent->GravityScale;
			MovementComponent->SetGravityScale(0.f);
			MovementComponent->Velocity = FVector::ZeroVector;
			bAirFrozen = true;
		}
	}
	
	_PrepareForAbilityEvent();
	
	HitContext = MakeShared<FYSAbilityHitContext>(GetOwningActorFromActorInfo());
	PlaybackContext = MakeShared<FYSPlaybackContext>();
	SetupPlayBack(TriggerEventData);

	// NeedReady 는 조준·레디 단계를 거치는 어빌리티다.
	// 여기서 커밋하면 조준하다 취소해도 쿨다운이 돌아버리므로,
	// 시전이 확정되는 노드(bCommitOnEnter)까지 커밋을 미룬다.
	bHasCommitted = ( AbilityTypes.Contains(EYSAbilityType::NeedReady) == false );

	if ( bHasCommitted )
	{
		CommitAbility(Handle, ActorInfo, ActivationInfo);
	}

	_AddWorldTag();
}

void UYSGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	AActor* OwnerActor = GetOwningActorFromActorInfo();

	if ( IsValid(OwnerActor) )
	{
		AYSCharacterPlayer* Character = Cast<AYSCharacterPlayer>(OwnerActor);

		if ( IsValid(Character) )
		{
			Character->RemoveStateToStateMachine(ChangeInputStateType);
		}
	}
	
	UYSAbilitySystemComponent* YSASC = Cast<UYSAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());

	if ( IsValid(YSASC) )
	{
		YSASC->OnGameplayTagStateChanged.RemoveDynamic(this, &UYSGameplayAbility::OnGameplayTagChanged);
		
		for ( const FActiveGameplayEffectHandle& RuntimeHandle : RuntimeData.GetActiveGameplayEffectHandle() )
		{
			YSASC->RemoveActiveGameplayEffect(RuntimeHandle);	
		}
		
		YSASC->RemoveActiveEffectsWithTags(BuffTags);
	}

	if ( IsValid(CurrentPlayback.Get()) )
	{
		CurrentPlayback.Get()->ReleaseMotionWarp();
	}
	
	if ( bBlockMovementDuringAbility )
	{
		UYSCharacterMovementComponent* MovementComponent = UYSCharacterMovementComponent::Get(OwnerActor);
		
		if ( IsValid(MovementComponent) )
		{
			MovementComponent->SetMovementBlocked(false);
		}
	}
	
	if ( bAirFrozen )
	{
		UYSCharacterMovementComponent* MovementComponent = UYSCharacterMovementComponent::Get(OwnerActor);

		if ( IsValid(MovementComponent) )
		{
			MovementComponent->SetGravityScale(CachedGravityScale);
		}

		bAirFrozen = false;
	}
	
	if ( UYSTargetingComponent* Targeting = UYSTargetingComponent::Get(OwnerActor) )
	{
		Targeting->EndTargeting(this);
	}

	_RemoveWorldTag();

	bHasCommitted = false;

	HitContext = nullptr;
	PlaybackContext = nullptr;
}

bool UYSGameplayAbility::TryTransition(const FGameplayTag& InputGameplayTag, EYSInputPhase InputPhase)
{
	if ( IsInputAcceptable() == false )
	{
		return false;
	}
	
	UYSAbilityPlaybackBase* AbilityPlaybackBase = CurrentPlayback.Get();
	if ( IsValid(AbilityPlaybackBase) == false )
	{
		return false;
	}
	
	
	// 바로 트랜지션 가능하면 Dispatch Next를 호출하는게 좋을 듯 싶다.. ( 원래는 어셉트 되고 예약이 된다면 return true를 시켰었음..
	// 사유는 인풋에 따른 처리가 부가적으로 필요한 경우에는 TryTransition에서 true를 리턴해서 인풋이 처리되었다는 것을 알려주는게 좋을 것 같아서임
	//  ( 예시로는 콤보 입력이 들어왔을 때, 콤보 입력이 처리된 건지, 아니면 인풋이 무시된 건지 구분하기 위해서 )
	return	AbilityPlaybackBase->TryAcceptInputTag();
}


void UYSGameplayAbility::NotifyPlaybackChainFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

UYSAbilityPlaybackBase* UYSGameplayAbility::GetPlaybackNode(int32 Index)
{
	const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& PlaybackArray = GetPlaybackArray();

	if ( PlaybackArray.IsValidIndex(Index) )
	{
		return PlaybackArray[Index];
	}

	return nullptr;
}

void UYSGameplayAbility::EnsureRuntimePlaybacks()
{
	if ( IsValid(PlaybackGraph) == false || PlaybackGraph->Playbacks.Num() == 0 )
	{
		return;
	}

	// 같은 컴파일 결과로 이미 사본을 떴으면 그대로 쓴다.
	// 개수만 비교하면 에디터에서 노드를 바꿔 다시 컴파일했을 때 낡은 사본을 계속 쓰게 된다.
	if ( RuntimePlaybacks.Num() > 0 && CachedPlaybackSerial == PlaybackGraph->CompileSerial )
	{
		return;
	}

	RuntimePlaybacks.Reset(PlaybackGraph->Playbacks.Num());

	for ( const TObjectPtr<UYSAbilityPlaybackBase>& Source : PlaybackGraph->Playbacks )
	{
		// Outer 를 this 로 둔다. 이 어빌리티 인스턴스와 수명을 같이하고,
		// 다른 액터의 어빌리티 인스턴스는 자기 사본을 따로 갖는다.
		RuntimePlaybacks.Add(IsValid(Source) ? DuplicateObject<UYSAbilityPlaybackBase>(Source, this) : nullptr);
	}

	CachedPlaybackSerial = PlaybackGraph->CompileSerial;
}

const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& UYSGameplayAbility::GetPlaybackArray()
{
	EnsureRuntimePlaybacks();

	// 그래프가 있어도 컴파일이 안 됐으면 비어 있다. 그때는 레거시로 떨어진다.
	if ( RuntimePlaybacks.Num() > 0 )
	{
		return RuntimePlaybacks;
	}

	return Playbacks;
}


void UYSGameplayAbility::_ProcessEvent(FGameplayEventData Payload)
{
	if (EventActionMap.Contains(Payload.EventTag) == false )
		return;
	const TArray<UYSAbilityEventAction*>& EventActions = EventActionMap[Payload.EventTag].GetEventActions();

	for ( UYSAbilityEventAction* Action : EventActions )
	{
		if ( IsValid(Action) )
		{
			Action->Execute(this, Payload);
		}
	}
}

void UYSGameplayAbility::_PrepareForAbilityEvent()
{
	for ( auto& EventAction : EventActionMap )
	{
		UAbilityTask_WaitGameplayEvent* Task = UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(this, EventAction.Key, nullptr, false);

		if ( IsValid(Task) )
		{
			Task->EventReceived.AddDynamic(this, &UYSGameplayAbility::_ProcessEvent);
			Task->ReadyForActivation();
		}
	}
}

void UYSGameplayAbility::_AddWorldTag()
{
	UYSWorldTagSubsystem* WorldTagSubsystem = UYSWorldTagSubsystem::Get(this);
	if (WorldTagSubsystem)
	{
		for (const FGameplayTag& Tag : WorldTagContainer)
		{
			WorldTagSubsystem->AddWorldTag(Tag);
		}
	}
}

void UYSGameplayAbility::_RemoveWorldTag()
{
	UYSWorldTagSubsystem* WorldTagSubsystem = UYSWorldTagSubsystem::Get(this);
	if (WorldTagSubsystem)
	{
		for (const FGameplayTag& Tag : WorldTagContainer)
		{
			WorldTagSubsystem->RemoveWorldTag(Tag);
		}
	}
}

void UYSGameplayAbility::OnMontagePlayed()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = true;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

void UYSGameplayAbility::OnMontageInterrupted()
{
	bool bReplicatedEndAbility = true;
	bool bWasCancelled = false;
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, bReplicatedEndAbility, bWasCancelled);
}

// 해당 부분 리팩토링 필요. 만약에 내부 데이터가 추가 필요한 경우에 날아와야 함.
void UYSGameplayAbility::SetupPlayBack(const FGameplayEventData* TriggerEventData)
{
	const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& PlaybackArray = GetPlaybackArray();

	if (!PlaybackArray.IsValidIndex(0))
	{
		return;
	}

	PlaybackContext->OwnerAbility = this;
	PlaybackContext->Instigator = GetOwningActorFromActorInfo();
	
	// 리플레이 전용...
	int32 StartIndex = 0;
	if ( TriggerEventData != nullptr && TriggerEventData->EventTag == YSTags::Event_Replay )
	{
		StartIndex = FMath::RoundToInt(TriggerEventData->EventMagnitude);
	}

	ActivePlayback(PlaybackArray.IsValidIndex(StartIndex) ? StartIndex : 0);
}

const FYSSkillInfo* UYSGameplayAbility::GetSkillInfo() const
{
	if ( SkillRow.IsNull() )
	{
		return nullptr;
	}
	
	return SkillRow.GetRow<FYSSkillInfo>(TEXT("context"));
}

#if UE_EDITOR
void UYSGameplayAbility::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	
	FName PropertyName = (PropertyChangedEvent.Property != NULL) ? PropertyChangedEvent.Property->GetFName() : NAME_None;
	
	if ( PropertyName == GET_MEMBER_NAME_CHECKED(UYSGameplayAbility, AbilityTypes) )
	{
		EventActionMap.Empty();
		
		for ( EYSAbilityType AbilityType : AbilityTypes )
		{
			switch (AbilityType )
			{
			case EYSAbilityType::None:
				break;
			case EYSAbilityType::MeleeAttack :
				{
					FYSEventPayload TraceStart;
					TraceStart.EventActions.Add(NewObject<UYSAbilityEventAction_StartTrace>(this));
					EventActionMap.Add(YSTags::Event_TraceStart, TraceStart);
			
					FYSEventPayload TraceEnd;
					TraceEnd.EventActions.Add(NewObject<UYSAbilityEventAction_StopTrace>(this));
					EventActionMap.Add(YSTags::Event_TraceEnd, TraceEnd);
					break;
				}
			case EYSAbilityType::RangedAttack :
				{
					FYSEventPayload SpawnActor;
					SpawnActor.EventActions.Add(NewObject<UYSAbilityEventAction_SpawnActor>(this));
					EventActionMap.Add(YSTags::Event_SpawnActor, SpawnActor);
					break;
				}
			case EYSAbilityType::Dash :
				{
					FYSEventPayload Velocity;
					Velocity.EventActions.Add(NewObject<UYSAbilityEventAction_ApplyVelocity>(this));
					EventActionMap.Add(YSTags::Event_ApplyVelocity, Velocity);
					break;
				}
			case EYSAbilityType::NeedCameraDirect :
				{
					FYSEventPayload CameraPush;
					CameraPush.EventActions.Add(NewObject<UYSAbilityEventAction_PushCamera>(this));
					EventActionMap.Add(YSTags::Event_PushCamera, CameraPush);
					
					FYSEventPayload CameraPop;
					CameraPop.EventActions.Add(NewObject<UYSAbilityEventAction_PopCamera>(this));
					EventActionMap.Add(YSTags::Event_PopCamera, CameraPop);
					break;
				}
			case EYSAbilityType::NeedReady :
				{
					FYSEventPayload Ready;
					Ready.EventActions.Add(NewObject<UYSAbilityEventAction_AimStart>(this));
					EventActionMap.Add(YSTags::Event_AimStart, Ready);
					
					FYSEventPayload EndReady;
					EndReady.EventActions.Add(NewObject<UYSAbilityEventAction_AimStop>(this));
					EventActionMap.Add(YSTags::Event_AimStop, EndReady);
					break;
				}
			case EYSAbilityType::UseResource :
				{
					FYSEventPayload Resource;
					Resource.EventActions.Add(NewObject<UYSAbilityEventAction_ConsumeResource>(this));
					EventActionMap.Add(YSTags::Event_ConsumeResource, Resource);
					break;
				}
			case EYSAbilityType::Hon_Only :
				{
					FYSEventPayload Record;
					Record.EventActions.Add(NewObject<UYSAbilityEventAction_SaveAbilityRecord>(this));
					EventActionMap.Add(YSTags::Event_Record, Record);
					break;
				}
			default: 
				break;
			}	
		}
	}
}
#endif
