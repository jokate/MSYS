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

// 해당 구조의 가장 큰 문제점은 어빌리티의 플레이 백을 의미하다보니 다른 어빌리티에서 동작 시, Race Condition이 발생할 수 있음.
// 사실 이게 다양한 상황에서의 전제가 있다고 가정한다면, 애니메이션이 좀 꼬일 수 있겠다는 생각은 드는 편.
// 그렇다면 규칙이 있음, 예를 들어서

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

protected :
	bool AreConditionsSatisfied(const TArray<FInstancedStruct>& Conditions) const;
	void ConsumeConditions(const TArray<FInstancedStruct>& Conditions) const;

protected :
	TSharedPtr<FYSPlaybackContext> CapturedContext;

private :
	// Play/EndPlay 마다 올린다. Task 가 동기로 전환을 확정하면 값이 바뀌어 순회를 멈춘다.
	int32 PlaySerial = 0;
};
