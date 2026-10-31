// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSStruct.h"
#include "Ability/AbilityComponent/YSPlaybackCondition.h"
#include "Ability/AbilityComponent/YSPlaybackTask.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "YSAbilityPlayback.generated.h"

class UYSPlaybackAction;
class UYSPlaybackTask;
class ULevelSequencePlayer;
class UAbilityTask_PlayMontageAndWait;
class ALevelSequenceActor;

// 주의: 어빌리티의 플레이백을 뜻하므로 다른 어빌리티에서 동작하면 Race Condition 이 생길 수 있다.

UCLASS(EditInlineNew, DefaultToInstanced, CollapseCategories)
class MNYS_API UYSAbilityPlaybackBase : public UObject
{
	GENERATED_BODY()

public :
	void Play(const TSharedPtr<FYSPlaybackContext>& Context);
	virtual void EndPlay();
	void ReleaseMotionWarp();

	bool HandleInput(const FGameplayTag& InputTag, EYSInputPhase InputPhase);
	void HandleInputWindowClosed();
	void OnHit(const TArray<FHitResult>& HitResults);
	void HandleContextTagChanged();

	/** Task 가 전환을 확정했을 때 부른다. -1 이면 체인 종료. */
	bool RequestTransition(int32 NextNodeIndex);

	/** 조건을 통과한 첫 경로로 전환한다. 통과한 경로가 없으면 아무것도 안 하고 false. */
	bool TryResolveRoutes(const TArray<FYSPlaybackRoute>& Routes);

	/** 재생 전에 본다. 첫 태스크가 즉시 분기면 통과할 경로가 있어야 참. 조건을 소비하지 않는다. */
	bool CanStartWith(const TSharedPtr<FYSPlaybackContext>& Context) const;

	const TSharedPtr<FYSPlaybackContext>& GetContext() const { return CapturedContext; }

	AActor* GetCurrentPlaybackTarget() const
	{
		if ( CapturedContext.IsValid() == false )
		{
			return nullptr;
		}
		return CapturedContext->Target;
	}

	UYSGameplayAbility* GetCurrentPlaybackOwningAbility() const
	{
		if ( CapturedContext.IsValid() == false )
		{
			return nullptr;
		}
		return CapturedContext->OwnerAbility;
	}

	AActor* GetCurrentPlaybackInstigator() const
	{
		if ( CapturedContext.IsValid() == false )
		{
			return nullptr;
		}
		return CapturedContext->Instigator;
	}

public :
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "YS | Playback", meta = (DisplayName = "진입 시 행동"))
	TArray<TObjectPtr<UYSPlaybackAction>> EnterActions;

	UPROPERTY(EditDefaultsOnly, Instanced, Category = "YS | Playback", meta = (DisplayName = "태스크 (배열 순서가 우선순위)"))
	TArray<TObjectPtr<UYSPlaybackTask>> Tasks;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Playback", meta = (DisplayName = "입력 불일치 시 체인 종료"))
	bool bEndChainOnUnmatchedInput = true;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Playback", meta = (DisplayName = "진입 시 커밋 (쿨다운 소모)"))
	bool bCommitOnEnter = false;

	/** 마지막 Play 에서 일이 실제로 시작됐는가. 진입 행동이 있거나 일이 시작된 태스크가 하나라도 있으면 참. 몽타주 재생이 실패한 노드는 거짓. */
	bool DidWork() const { return bDidWork; }

protected :
	static bool AreConditionsSatisfied(const TArray<FInstancedStruct>& Conditions, const TSharedPtr<FYSPlaybackContext>& Context);
	void ConsumeConditions(const TArray<FInstancedStruct>& Conditions) const;

protected :
	TSharedPtr<FYSPlaybackContext> CapturedContext;

private :
	// Play/EndPlay 마다 올린다. Task 가 동기로 전환을 확정하면 값이 바뀌어 순회를 멈춘다.
	int32 PlaySerial = 0;

	bool bDidWork = false;
};
