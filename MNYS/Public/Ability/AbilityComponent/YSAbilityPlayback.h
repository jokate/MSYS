// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSStruct.h"
#include "Ability/AbilityComponent/YSPlaybackCondition.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "YSAbilityPlayback.generated.h"

class ULevelSequencePlayer;
class UAbilityTask_PlayMontageAndWait;
class ALevelSequenceActor;
/**
 * 
 */
UENUM(BlueprintType)
enum class EYSAbilityPlaybackType : uint8
{
	None UMETA(DisplayName = "아무것도 재생 안함"),
	Montage UMETA(DisplayName = "몽타주 재생"),
	Sequence UMETA(DisplayName = "시퀀스 재생"),
};

USTRUCT()
struct FYSPlaybackEdge
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "YS | Transition", meta = (DisplayName = "발화 조건"))
	EYSPlaybackEvent RequiredResult = EYSPlaybackEvent::Completed;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Transition", meta = (DisplayName = "전환 조건", BaseStruct = "/Script/MNYS.YSPlaybackCondition", ExcludeBaseStruct))
	TArray<FInstancedStruct> TransitionConditions;
	
	// -1 = 체인 종료 (어빌리티 EndAbility)
	UPROPERTY(EditDefaultsOnly, Category = "YS | Transition", meta = (DisplayName = "다음 노드 인덱스 (-1: 종료)"))
	int32 NextNodeIndex = INDEX_NONE;
};

// 해당 구조의 가장 큰 문제점은 어빌리티의 플레이 백을 의미하다보니 다른 어빌리티에서 동작 시, Race Condition이 발생할 수 있음.
// 사실 이게 다양한 상황에서의 전제가 있다고 가정한다면, 애니메이션이 좀 꼬일 수 있겠다는 생각은 드는 편.
// 그렇다면 규칙이 있음, 예를 들어서 

UCLASS(EditInlineNew, DefaultToInstanced, CollapseCategories)
class MNYS_API UYSAbilityPlaybackBase : public UObject
{
	GENERATED_BODY()
	
public : 
	void Play(const TSharedPtr<FYSPlaybackContext>& Context);
	void ReleaseMotionWarp();
	bool DispatchNext(EYSPlaybackEvent Event, bool bIsEvaluate = false);
	void OnHit(const TArray<FHitResult>& HitResults);
	
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
	
	virtual void EndPlay();
	
	bool TryAcceptContextTag(const FGameplayTag& InputTag, EYSInputPhase InputPhase);
	
protected :
	UFUNCTION()
	void OnMontagePlayed();

	UFUNCTION()
	void OnMontageInterrupted();
	
	virtual void SetupMontage();
	
	virtual void SetPlayback(TSharedPtr<FYSPlaybackContext> Context);
	
	UFUNCTION()
	void OnSequencePlayed();
	
	virtual void SetupSequence();

	virtual void ProcessContextBeforePlay() {};
	
public : 
	UPROPERTY(EditDefaultsOnly, Category = "YS | Ability Playback Type")
	EYSAbilityPlaybackType PlaybackType = EYSAbilityPlaybackType::Montage;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Sequence", meta = (DisplayName = "시퀀스 설정", 	EditCondition = "PlaybackType == EYSAbilityPlaybackType::Sequence"))
	FYSSequencePlaySettings SequenceSettings;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Montage To Play", meta = (DisplayName = "재생할 몽타주 정보", EditCondition = "PlaybackType == EYSAbilityPlaybackType::Montage", BaseStruct = "/Script/MNYS.YSMontageSelector", ExcludeBaseStruct))
	FInstancedStruct MontageSelector;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Transitions", meta = (DisplayName = "다음 플레이백 전환"))
	TArray<FYSPlaybackEdge> Transitions;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Transitions", meta = (DisplayName = "즉시 전환 가능 여부"))
	bool bImmediateTransition = false;

	/**
	 * 입력이 어느 엣지와도 맞지 않았을 때 체인을 끊을지 여부.
	 *
	 * 콤보 노드는 끊는 게 맞다 — 엉뚱한 입력이 오면 콤보가 실패한 것이다.
	 * 하지만 조준 대기처럼 무관한 입력(이동·회피)이 계속 흘러드는 노드에서는
	 * 꺼야 한다. 켜두면 조준 중 한 발짝만 움직여도 어빌리티가 끝난다.
	 *
	 * 몽타주 완료 같은 다른 이벤트는 이 값과 무관하게 항상 체인을 끝낸다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "YS | Transitions", meta = (DisplayName = "입력 불일치 시 체인 종료"))
	bool bEndChainOnUnmatchedInput = true;

	/**
	 * 이 노드에 진입할 때 어빌리티를 커밋(쿨다운·비용 소모)한다.
	 *
	 * 어빌리티 타입에 NeedReady 가 있으면 활성 시점에는 커밋하지 않으므로,
	 * 실제 시전이 확정되는 노드(발사 몽타주 등)에 이 값을 켜야 한다.
	 * 켜지 않으면 그 어빌리티는 쿨다운을 영영 소모하지 않는다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "YS | Transitions", meta = (DisplayName = "진입 시 커밋 (쿨다운 소모)"))
	bool bCommitOnEnter = false;

protected : 
	// Play() 시점에 캡처 — 콜백에서 컨텍스트 참조용
	TSharedPtr<FYSPlaybackContext> CapturedContext;
	
private :
	UPROPERTY()
	TObjectPtr<ALevelSequenceActor> ActiveSequenceActor = nullptr;
	
	UPROPERTY()
	TObjectPtr<UAbilityTask_PlayMontageAndWait> PlayMontageAndWaitTask = nullptr;
	
	UPROPERTY()
	TObjectPtr<ULevelSequencePlayer> LevelSequencePlayer = nullptr;
	
};

UCLASS(DisplayName = "히트 첫번째 타겟 기준 플레이 백")
class MNYS_API UYSAbilityPlayback_FirstHitTarget : public UYSAbilityPlaybackBase
{
	GENERATED_BODY()
	
protected :
	virtual void ProcessContextBeforePlay() override;
};


UCLASS(DisplayName = "플레이 백 시작 시, 버프 해제 / 버프 추가")
class MNYS_API UYSAbilityPlayback_ReleaseBuff : public UYSAbilityPlaybackBase
{
	GENERATED_BODY()
	
public : 
	UPROPERTY(EditAnywhere, Category = "YS | ReleaseBuff")
	FGameplayTagContainer BuffTags;
	
protected :  
	virtual void ProcessContextBeforePlay() override;
};