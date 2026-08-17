// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Components/ActorComponent.h"
#include "General/YSEnum.h"
#include "General/YSStruct.h"
#include "YSInputStateMachineComponent.generated.h"


class UYSAbilitySystemComponent;
struct FYSCommandSequence;
class UYSInputStates;


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnRawInputAccepted, const FGameplayTag&, InputTag, EYSInputPhase, InputPhase);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSInputStateMachineComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	// Sets default values for this component's properties
	static UYSInputStateMachineComponent* Get(AActor* Owner);
	UYSInputStateMachineComponent();
	virtual void AcceptInput(const FGameplayTag& Tag, EYSInputPhase InputPhase);

	/**
	 * 상태 해석과 어빌리티 라우팅을 거치지 않고 원본 태그 그대로 나가는 통로.
	 *
	 * 어느 상태에서든 의미가 같고 어빌리티가 아닌 입력을 위한 것이다 —
	 * 혼의 세이브처럼 몽타주·쿨다운·코스트·상태 전환이 전부 없는 조작.
	 *
	 * 어빌리티로 만들지 않는 이유가 편의만은 아니다. 어빌리티 경로로 보내면
	 * (1) State 11종 × 입력만큼 태그 조합을 다 등록해야 하고,
	 * (2) 활성 콤보의 TryTransition 이 먼저 이 입력을 받아 맞는 엣지를 못 찾고
	 *     bEndChainOnUnmatchedInput 에 걸려 콤보를 끊는다. 저장하려고 누른 키가
	 *     저장하려던 콤보를 죽이는 자기모순이 된다.
	 *
	 * 입력 차단(BlockInput) 은 존중한다 — 경직 중에는 이 통로도 막힌다.
	 */
	UPROPERTY(BlueprintAssignable, Category = "YS | Input", meta = (DisplayName = "원본 입력 수신"))
	FOnRawInputAccepted OnRawInputAccepted;
	
	virtual void AddStateStack(EYSInputStatesType State);
	virtual void RemoveStateStack(EYSInputStatesType State);
	
	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, enum ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	
#pragma region History Recording
	void RecordInputHistory(const FGameplayTag& Tag, EYSInputPhase InputPhase);
	void TrimInputHistory();
	bool ContainsInputHistory(const FGameplayTag& Tag, EYSInputPhase InputPhase) const;

	void ConsumeInputHistory(const FGameplayTag& Tag, EYSInputPhase InputPhase);

protected :
	int32 FindLiveHistoryIndex(const FGameplayTag& Tag, EYSInputPhase InputPhase) const;

public :
#pragma endregion

	/**
	 * 해당 입력이 지금 물리적으로 눌려 있는가.
	 *
	 * 히스토리와 달리 소비되지 않는 상태 질의다. 그래서 연사 루프처럼
	 * "누르고 있는 동안 반복"을 매 프레임 입력 없이 표현할 수 있다.
	 *
	 * 상태 접두가 붙지 않은 원본 태그(Input.Aim)로 묻는다 — 아래 HeldInputTags 주석 참고.
	 */
	bool IsInputHeld(const FGameplayTag& RawInputTag) const { return HeldInputTags.Contains(RawInputTag); }
	
protected:
	void ResetInputTags()
	{
		ComboInputTags.Empty();
	}
	
	virtual void TransitionState(EYSInputStatesType NewInputState);
	
	// Called when the game starts

	template<typename T>
	void AddState();

	FGameplayTag FindBestCombo(const FGameplayTag& Tag);

	UFUNCTION()
	void OnTagUpdated(const FGameplayTag& Tag, bool bActive);
	
	void TrimCommandInput();


public :
	// 현재 InputState에 대한 요청들을 담아둡니다. 만약 Transition이 불가능하더라도 요청은 남습니다.
	// 만약 제거 될 때 최상단의 State로 변경됩니다.
	UPROPERTY(VisibleAnywhere)
	TArray<EYSInputStatesType> InputStateRequests;
	
	UPROPERTY()
	TObjectPtr<UYSInputStates> CurrentInputState;

	UPROPERTY()
	TMap<EYSInputStatesType, UYSInputStates*> InputStates;

protected:
	UPROPERTY()
	FTimerHandle ComboTimerHandle;
	
	UPROPERTY()
	FTimerHandle InputTrimTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FYSTagHistory> ComboInputTags;
	
	// 내 생각에는 만약 InputHistory를 단일 저장한다 ( 약간 Map의 형식이 맞겠지.. )
	UPROPERTY()
	TArray<FYSInputHistory> InputHistories;
	
	UPROPERTY(VisibleAnywhere, Category = "YS | Input", meta = (DisplayName = "현재 눌려 있는 입력"))
	TSet<FGameplayTag> HeldInputTags;

	// 해당 값은 조작감에 따라서 처리되기로 합시다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float InputProcessingTime = 0.5f;
	
	// Tick이나 타이머 돌면서 Trim 처리.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float InputExpirationTime = 0.33f;

private :
	TArray<const FYSCommandSequence*> AllCommandSequence;

	bool bIsInputBlocked;
	TWeakObjectPtr<UYSAbilitySystemComponent> AbilitySystemComponent;
};
