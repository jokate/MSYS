// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/YSGameplayAbility.h"

#include "AbilitySystemComponent.h"
#include "YSAbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Ability/EventAction/YSAbilityEventAction.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Ability/AbilityComponent/YSPlaybackGraphAsset.h"
#include "Character/Components/YSCharacterMovementComponent.h"
#include "Character/Components/YSTargetingComponent.h"
#include "General/YSGameplayTag.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"
#include "Library/YSBlueprintFunctionLibrary.h"
#include "Subsystem/YSWorldTagSubsystem.h"

FYSAbilityHitContext::FYSAbilityHitContext(AActor* Instigator, UYSGameplayAbility* InOwningAbility)
	: InstigatorActor(Instigator), OwningAbility(InOwningAbility)
{
}

void FYSAbilityHitContext::ReportHit(UObject* HitObject, const TArray<FHitResult>& HitResults)
{
	UYSGameplayAbility* Ability = OwningAbility.Get();

	if ( IsValid(Ability) == false )
	{
		return;
	}

	for ( const FHitResult& HitResult : HitResults )
	{
		AddHitActor(HitResult.GetActor());
		UpdateHitResult(HitObject, HitResult);
	}

	if ( UYSAbilityPlaybackBase* Playback = Ability->GetCurrentPlayback() )
	{
		Playback->OnHit(HitResults);
	}
}

void FYSAbilityHitContext::OnOwnerEnded()
{
	EndedEffectScale = GetEffectScale();
	OwningAbility = nullptr;
	HitActors.Reset();
	RecentHitResults.Reset();

	OnOwnerEndedEvent.Broadcast();
	OnOwnerEndedEvent.Clear();
}

float FYSAbilityHitContext::GetEffectScale() const
{
	const UYSGameplayAbility* Ability = OwningAbility.Get();
	return IsValid(Ability) ? Ability->GetEffectScale() : EndedEffectScale;
}

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

	// 범위 밖 · null 은 데이터 오류다. 그냥 돌려보내면 체인을 이어갈 주체가 없어 어빌리티가 켜진 채 남는다.
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

	// 조준하다 취소해도 쿨다운이 돌지 않게, 시전이 확정되는 노드까지 커밋을 미룰 수 있다.
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

	if ( const UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(OwnerActor) )
	{
		const FActiveGameplayEffectHandle StateHandle = InputStateMachine->ApplyStateEffect(InputState);

		if ( StateHandle.IsValid() )
		{
			AddRuntimeEffectSpecHandle(StateHandle);
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
			MovementComponent->SuspendGravity(this);
			MovementComponent->Velocity = FVector::ZeroVector;

			AddEndCleanup([WeakMovement = TWeakObjectPtr<UYSCharacterMovementComponent>(MovementComponent), this]()
			{
				if ( UYSCharacterMovementComponent* Movement = WeakMovement.Get() )
				{
					Movement->ResumeGravity(this);
				}
			});
		}
			
		YSASC->AddAirUsage(Handle);
	}
	
	_PrepareForAbilityEvent();
	
	HitContext = MakeShared<FYSAbilityHitContext>(GetOwningActorFromActorInfo(), this);
	PlaybackContext = MakeShared<FYSPlaybackContext>();
	bHasCommitted = false;
	bPlaybackDidWork = false;

	// 체인이 동기로 끝나면 EndAbility 가 먼저 도므로 월드 태그를 앞서 더한다 (뒤에 더하면 짝이 되는 제거가 없어 영구히 남음).
	_AddWorldTag();
	SetupPlayBack(TriggerEventData);

	// 일이 시작된 노드 없이 체인이 끝났다(빈 노드 · 범위 밖 · null · 몽타주 재생 실패). 아무것도 안 한 어빌리티에 쿨다운을 걸지 않는다.
	if ( IsActive() == false && bPlaybackDidWork == false )
	{
		return;
	}

	if ( bHasCommitted == false && HasCommitOnEnterNode() == false )
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

	UYSAbilitySystemComponent* YSASC = Cast<UYSAbilitySystemComponent>(GetAbilitySystemComponentFromActorInfo());

	if ( IsValid(YSASC) )
	{
		YSASC->OnGameplayTagStateChanged.RemoveDynamic(this, &UYSGameplayAbility::OnGameplayTagChanged);
		// 구독을 끊었으니 창을 닫는 알림은 더 오지 않는다. 여기서 닫지 않으면 열린 채로 남는다.
		SetInputAcceptable(false);

		
		YSASC->RemoveActiveEffectsWithTags(BuffTags);
	}

	// 실행 중 새 등록이 생겨도 이번 목록만 돌도록 옮겨서 돈다. ASC 유무와 무관하게 돌아야 하므로 위 블록 밖에 둔다.
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
		// 시퀀스 플레이어는 어빌리티가 끝나도 스스로 멈추지 않는다. 안 끊으면 OnFinished 가 null 컨텍스트로 돌아오거나 bPauseAtEnd 로 굳는다.
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
	
	if ( UYSTargetingComponent* Targeting = UYSTargetingComponent::Get(OwnerActor) )
	{
		Targeting->EndTargeting(this);
	}

	_RemoveWorldTag();

	bHasCommitted = false;

	if ( HitContext.IsValid() )
	{
		if ( bWasCancelled || bCancelling )
		{
			HitContext->ReleaseAllLinkedActors(EYSReleaseReason::Cancelled);
		}

		HitContext->OnOwnerEnded();
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
	
	
	// 입력 처리 여부를 TryTransition 의 반환으로 알린다 (예: 콤보 입력이 처리됐는지 무시됐는지 구분).
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
		RuntimePlaybacks.Reset();
		return;
	}

	// 같은 컴파일 결과로 이미 사본을 떴으면 그대로 쓴다. 개수만 비교하면 노드를 바꿔 다시 컴파일해도 낡은 사본을 쓴다.
	if ( RuntimePlaybacks.Num() > 0 && CachedPlaybackSerial == PlaybackGraph->CompileSerial )
	{
		return;
	}

	RuntimePlaybacks.Reset(PlaybackGraph->Playbacks.Num());

	for ( const TObjectPtr<UYSAbilityPlaybackBase>& Source : PlaybackGraph->Playbacks )
	{
		// Outer 는 this — 어빌리티 인스턴스마다 자기 사본을 갖고 수명을 같이한다.
		RuntimePlaybacks.Add(IsValid(Source) ? DuplicateObject<UYSAbilityPlaybackBase>(Source, this) : nullptr);
	}

	CachedPlaybackSerial = PlaybackGraph->CompileSerial;
}

bool UYSGameplayAbility::CanStartPlayback(const TSharedPtr<FYSPlaybackContext>& Context) const
{
	// 사본이 아니라 그래프 원본을 본다. 발동 전이라 사본이 없을 수 있고, 조건만 읽는다.
	const UYSAbilityPlaybackBase* First = IsValid(PlaybackGraph) && PlaybackGraph->Playbacks.Num() > 0 ? PlaybackGraph->Playbacks[0].Get() : nullptr;

	return IsValid(First) && First->CanStartWith(Context);
}

const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& UYSGameplayAbility::GetPlaybackArray()
{
	EnsureRuntimePlaybacks();

	return RuntimePlaybacks;
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

bool UYSGameplayAbility::HasCommitOnEnterNode()
{
	for ( const TObjectPtr<UYSAbilityPlaybackBase>& Playback : GetPlaybackArray() )
	{
		if ( IsValid(Playback) && Playback->bCommitOnEnter )
		{
			return true;
		}
	}

	return false;
}
