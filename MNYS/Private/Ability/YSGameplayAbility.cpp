// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/YSGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "YSAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Ability/EventAction/YSAbilityEventAction.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
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
	
		PlaybackBase->DispatchNext(EYSPlaybackEvent::OnCheckContextTag, true);
	}
}

void UYSGameplayAbility::ActivePlayback(int32 Index)
{	
	if ( Playbacks.IsValidIndex(Index) == false )
		return;

	UYSAbilityPlaybackBase* Playback = Playbacks[Index];

	if ( IsValid(CurrentPlayback.Get()))
	{
		CurrentPlayback->EndPlay();	
	}
	
	if ( IsValid(Playback) == false )
		return;

	CurrentPlayback = Playback;

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
	
	// 조준을 정리한다.
	// 몽타주 NotifyEnd 로 Event.AimStop 을 쏴도 이 시점엔 WaitGameplayEvent 태스크가 이미 죽어
	// 이벤트가 버려진다. 게다가 InstancedPerActor 라 어빌리티 인스턴스가 살아남아
	// 타게팅 컴포넌트의 약참조 안전망도 걸리지 않는다. 그래서 여기서 직접 끊는다.
	// EndTargeting 이 요청자를 대조하므로 조준을 쓰지 않은 어빌리티에는 아무 영향이 없다.
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
	return	AbilityPlaybackBase->TryAcceptContextTag(InputGameplayTag, InputPhase);
}


void UYSGameplayAbility::NotifyPlaybackChainFinished()
{
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

UYSAbilityPlaybackBase* UYSGameplayAbility::GetPlaybackNode(int32 Index)
{
	if ( Playbacks.IsValidIndex(Index) )
	{
		return Playbacks[Index];
	}

	return nullptr;
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

void UYSGameplayAbility::SetupPlayBack(const FGameplayEventData* TriggerEventData)
{
	if (!Playbacks.IsValidIndex(0))
	{
		return;
	}

	PlaybackContext->OwnerAbility = this;
	PlaybackContext->Instigator = GetOwningActorFromActorInfo();

	ActivePlayback(0);
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
			default: 
				break;
			}	
		}
	}
}
#endif
