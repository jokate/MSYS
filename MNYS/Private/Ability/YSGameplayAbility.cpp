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

#if UE_EDITOR
#include "Misc/DataValidation.h"
#endif

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
			PlaybackBase->HandleInputWindowClosed();
		}
	}
}

void UYSGameplayAbility::AddRuntimeEffectSpecHandle(const FActiveGameplayEffectHandle& Handle)
{
	AddEndCleanup([WeakASC = TWeakObjectPtr<UAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo()), Handle]()
	{
		if ( UAbilitySystemComponent* ASC = WeakASC.Get() )
		{
			ASC->RemoveActiveGameplayEffect(Handle);
		}
	});
}

void UYSGameplayAbility::ActivePlayback(int32 Index)
{
	const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& PlaybackArray = GetPlaybackArray();

	UYSAbilityPlaybackBase* Playback = PlaybackArray.IsValidIndex(Index) ? PlaybackArray[Index].Get() : nullptr;

	// 끝으로 가는 정상 경로는 INDEX_NONE → NotifyPlaybackChainFinished 다. 여기 오는 범위 밖 · null 은 데이터 오류이고,
	// 그냥 돌려보내면 체인을 이어갈 주체가 없어 어빌리티가 켜진 채 남는다.
	// 현재 노드의 EndPlay 는 EndAbility 가 부르므로 그 앞에서 판정한다.
	if ( IsValid(Playback) == false )
	{
		UE_LOG(LogTemp, Warning, TEXT("[Playback] %s: 노드 %d 를 켤 수 없다(범위 밖 또는 null, 노드 %d개). 어빌리티를 종료한다."),
			*GetName(), Index, PlaybackArray.Num());
		NotifyPlaybackChainFinished();
		return;
	}

	if ( IsValid(CurrentPlayback.Get()))
	{
		CurrentPlayback->EndPlay();
	}

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

	// 일이 시작됐는지는 Play 가 끝난 뒤에 안다. 몽타주 재생 실패처럼 Play 안에서 체인이 끝나도 이 노드의 몫은 센다.
	bPlaybackDidWork |= Playback->DidWork();
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

	// 인스턴스가 재사용된다. 앞 사용에서 입력 창이 열린 채 끝났어도 이번 사용은 닫힌 채로 시작한다.
	SetInputAcceptable(false);
	EndCleanups.Reset();

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

	UYSCharacterMovementComponent* MovementComponent = UYSCharacterMovementComponent::Get(OwnerActor);

	// 지상에서 켜면 의미가 없고, 복구 대상만 늘어난다.
	if ( IsValid(MovementComponent) && MovementComponent->IsFalling() )
	{
		if ( bFreezeInAirDuringAbility )
		{
			CachedGravityScale = MovementComponent->GravityScale;
			MovementComponent->SetGravityScale(0.f);
			MovementComponent->Velocity = FVector::ZeroVector;
			bAirFrozen = true;	
		}
			
		YSASC->AddAirUsage(Handle);
	}
	
	_PrepareForAbilityEvent();
	
	HitContext = MakeShared<FYSAbilityHitContext>(GetOwningActorFromActorInfo());
	PlaybackContext = MakeShared<FYSPlaybackContext>();
	bHasCommitted = false;
	bPlaybackDidWork = false;

	// SetupPlayBack 안에서 체인이 동기로 끝나면 EndAbility 가 먼저 돈다.
	// 월드 태그를 그 뒤에 더하면 짝이 되는 제거가 없어 영구히 남으므로 앞서 더한다.
	_AddWorldTag();
	SetupPlayBack(TriggerEventData);

	// 일이 시작된 노드가 하나도 없이 체인이 끝났다(빈 노드 · 범위 밖 · null · 몽타주 재생 실패). 아무것도 안 한 어빌리티에 쿨다운을 걸지 않는다.
	// 즉발 어빌리티는 진입 행동이나 시작된 태스크가 있어 일이 있는 노드로 세므로 그대로 커밋한다.
	if ( IsActive() == false && bPlaybackDidWork == false )
	{
		return;
	}

	// NeedReady 는 조준·레디 단계를 거치는 어빌리티다.
	// 여기서 커밋하면 조준하다 취소해도 쿨다운이 돌아버리므로,
	// 시전이 확정되는 노드(bCommitOnEnter)까지 커밋을 미룬다.
	// 0번 노드가 bCommitOnEnter 면 SetupPlayBack 안에서 이미 커밋됐으므로 다시 하지 않는다.
	if ( AbilityTypes.Contains(EYSAbilityType::NeedReady) == false && bHasCommitted == false )
	{
		bHasCommitted = true;
		CommitAbility(Handle, ActorInfo, ActivationInfo);
	}
}

void UYSGameplayAbility::EndAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateEndAbility, bool bWasCancelled)
{
	// 취소 방송 중 몽타주 태스크가 '중단'으로 먼저 끝내면, CancelAbility 의 EndAbility 가 한 번 더 온다.
	if ( IsEndAbilityValid(Handle, ActorInfo) == false )
	{
		return;
	}

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);

	// 스코프 락으로 미뤄졌으면 그 호출이 다시 들어올 때 정리한다.
	if ( IsActive() )
	{
		return;
	}

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
		// 구독을 끊었으니 창을 닫는 알림은 더 오지 않는다. 여기서 닫지 않으면 열린 채로 남는다.
		SetInputAcceptable(false);

		
		YSASC->RemoveActiveEffectsWithTags(BuffTags);
	}

	// 어빌리티가 건 큐 · 카메라 모드 등. 실행 중 새 등록이 생겨도 이번 목록만 돌도록 옮겨서 돈다.
	// ASC 유무와 무관하게 돌아야 하므로 위 블록 밖에 둔다. 대상이 사라졌는지는 각 람다가 본다.
	{
		TArray<TFunction<void()>> Cleanups = MoveTemp(EndCleanups);
		EndCleanups.Reset();

		for ( int32 i = Cleanups.Num() - 1; i >= 0; --i )
		{
			Cleanups[i]();
		}
	}

	if ( IsValid(CurrentPlayback.Get()) )
	{
		CurrentPlayback.Get()->ReleaseMotionWarp();
		// 시퀀스 플레이어는 어빌리티 태스크가 아니라 어빌리티가 끝나도 스스로 멈추지 않는다.
		// 여기서 끊지 않으면 OnFinished 가 null 컨텍스트로 되돌아오거나, bPauseAtEnd 상태로 영영 굳는다.
		CurrentPlayback.Get()->EndPlay();
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

	// 취소면 이번 사용이 남긴 연결 액터도 같이 걷는다.
	if ( ( bWasCancelled || bCancelling ) && HitContext.IsValid() )
	{
		HitContext->ReleaseAllLinkedActors(EYSReleaseReason::Cancelled);
	}

	HitContext = nullptr;
	PlaybackContext = nullptr;
}

void UYSGameplayAbility::CancelAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
	const FGameplayAbilityActivationInfo ActivationInfo, bool bReplicateCancelAbility)
{
	TGuardValue<bool> CancellingGuard(bCancelling, true);
	Super::CancelAbility(Handle, ActorInfo, ActivationInfo, bReplicateCancelAbility);
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
	return	AbilityPlaybackBase->HandleInput(InputGameplayTag, InputPhase);
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
		// 그래프도 레거시 배열도 비었으면 클래스 기본으로 한 번 채운다.
		if ( RuntimePlaybacks.Num() == 0 && Playbacks.Num() == 0 )
		{
			BuildDefaultPlaybacks(RuntimePlaybacks);

			if ( RuntimePlaybacks.Num() > 0 )
			{
				UE_LOG(LogTemp, Verbose, TEXT("[Playback] %s: 그래프 · 레거시가 비어 클래스 기본 플레이백(%d 노드)을 쓴다."), *GetName(), RuntimePlaybacks.Num());
			}
		}
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

EDataValidationResult UYSGameplayAbility::IsDataValid(FDataValidationContext& Context) const
{
	EDataValidationResult Result = Super::IsDataValid(Context);

	bool bHasActionType = false;

	for ( EYSAbilityType AbilityType : AbilityTypes )
	{
		if ( AbilityType != EYSAbilityType::None )
		{
			bHasActionType = true;
			break;
		}
	}

	// PostEditChangeProperty 를 안 거치고 AbilityTypes 만 써 넣으면 이 상태가 된다 (리플렉션 직접 쓰기, 외부 툴).
	if ( bHasActionType && EventActionMap.IsEmpty() )
	{
		Context.AddError(FText::FromString(TEXT("AbilityTypes 가 있는데 EventActionMap 이 비어 있다. AbilityTypes 를 에디터에서 다시 지정해 EventActionMap 을 재생성할 것.")));
		Result = EDataValidationResult::Invalid;
	}

	return Result;
}
#endif
