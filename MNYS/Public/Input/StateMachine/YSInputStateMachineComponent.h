// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ActiveGameplayEffectHandle.h"
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
	static UYSInputStateMachineComponent* Get(AActor* Owner);
	UYSInputStateMachineComponent();
	virtual void AcceptInput(const FGameplayTag& Tag, EYSInputPhase InputPhase);

	/** 상태 해석 · 어빌리티 라우팅 없이 원본 태그 그대로 나가는 통로. 혼의 세이브처럼 몽타주 · 쿨다운 · 상태 전환이 없는 조작용이다. 어빌리티 경로로 보내면 활성 콤보가 먼저 받아 bEndChainOnUnmatchedInput 에 걸려 콤보를 끊는다. 입력 차단은 존중한다(경직 중엔 막힌다). */
	UPROPERTY(BlueprintAssignable, Category = "YS | Input", meta = (DisplayName = "원본 입력 수신"))
	FOnRawInputAccepted OnRawInputAccepted;

	/** 원본 통로 구독자가 이 태그를 소비한다고 등록한다. 등록된 태그는 원본 방송까지만 가고 커맨드 버퍼 · 상태 해석 · 어빌리티로 흐르지 않는다 (활성 콤보가 받으면 체인이 끊긴다). 등록자가 사라지면 무효. */
	void ClaimRawInput(const FGameplayTag& Tag, const UObject* Claimer);
	void ReleaseRawInputs(const UObject* Claimer);
	bool IsRawInputClaimed(const FGameplayTag& Tag) const;

	/** 정의 표에서 이 상태의 GE 를 찾아 주인에게 건다. 건 쪽이 받은 핸들로 뗀다. 상태가 비었거나 표에 GE 가 없으면 빈 핸들. */
	FActiveGameplayEffectHandle ApplyStateEffect(FName State, const UObject* SourceObject = nullptr) const;

	/** 이 상태 요청이 스택에 있는가. 맨 위가 아니어도 참이다. */
	bool IsStateRequested(FName State) const { return InputStateRequests.Contains(State); }

	UYSInputStates* FindState(FName State) const;

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

	/** 해당 입력이 지금 물리적으로 눌려 있는가. 소비되지 않는 상태 질의라 연사 루프를 입력 없이 표현할 수 있다. 상태 접두 없는 원본 태그(Input.Aim)로 묻는다. */
	bool IsInputHeld(const FGameplayTag& RawInputTag) const { return HeldInputTags.Contains(RawInputTag); }

	// 뗌을 흘리지 않고 홀드 목록에서만 뺀다. 축에서 합성해 뗌이 없는 방향 태그용.
	void ClearInputHeld(const FGameplayTag& RawInputTag) { HeldInputTags.Remove(RawInputTag); }

	/** 요청자가 있는 동안 누름을 처리하지 않고 첫 누름 하나만 보관했다가, 마지막 요청자가 빠질 때 다시 흘린다 (그 사이 뗐으면 뗌까지). 히트스톱 선입력용. */
	void PushInputHold(const UObject* Requester);
	void PopInputHold(const UObject* Requester);

protected:
	bool HoldInputIfRequested(const FGameplayTag& Tag, EYSInputPhase InputPhase);
	void FlushHeldPress();
	void DispatchInput(const FGameplayTag& Tag, EYSInputPhase InputPhase);

	TArray<TWeakObjectPtr<const UObject>> InputHoldRequesters;
	TMultiMap<FGameplayTag, TWeakObjectPtr<const UObject>> RawInputClaims;
	FGameplayTag HeldPress;
	bool bHeldPressReleased = false;

protected:
	void ResetInputTags()
	{
		ComboInputTags.Empty();
	}
	
	void TransitionState(FName NewInputState);

	FGameplayTag FindBestCombo(const FGameplayTag& Tag);

	UFUNCTION()
	void OnTagUpdated(const FGameplayTag& Tag, bool bActive);
	
	void TrimCommandInput();


private :
	// 상태 스택은 GE(UYSInputStateGEComponent)로만 바뀐다. 요청자 세기와 '건 쪽이 뗀다'를 GE 가 맡는다.
	friend class UYSInputStateGEComponent;

	void AddStateStack(FName State);
	void RemoveStateStack(FName State);

public :
	// 늘 깔려 있고 빠지지 않는 상태. 표에 행이 있어야 한다.
	UPROPERTY(EditDefaultsOnly, Category = "YS | Input", meta = (DisplayName = "바닥 상태", GetOptions = "MNYS.YSDeveloperSettings.GetInputStateOptions"))
	FName BaseInputState = TEXT("Idle");

	// 창 규칙의 '조준 중엔 조준 상태로 넘길 입력'이 넘어갈 상태.
	UPROPERTY(EditDefaultsOnly, Category = "YS | Input", meta = (DisplayName = "조준 상태", GetOptions = "MNYS.YSDeveloperSettings.GetInputStateOptions"))
	FName AimInputState = TEXT("Aim");

	// 현재 InputState 에 대한 요청들을 담아둔다. Transition 이 불가능해도 요청은 남고, 제거될 때 최상단 State 로 바뀐다.
	UPROPERTY(VisibleAnywhere)
	TArray<FName> InputStateRequests;
	
	UPROPERTY()
	TObjectPtr<UYSInputStates> CurrentInputState;

	UPROPERTY()
	TMap<FName, TObjectPtr<UYSInputStates>> InputStates;

protected:
	UPROPERTY()
	FTimerHandle ComboTimerHandle;
	
	UPROPERTY()
	FTimerHandle InputTrimTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TArray<FYSTagHistory> ComboInputTags;

	UPROPERTY()
	TArray<FYSInputHistory> InputHistories;
	
	UPROPERTY(VisibleAnywhere, Category = "YS | Input", meta = (DisplayName = "현재 눌려 있는 입력"))
	TSet<FGameplayTag> HeldInputTags;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float InputProcessingTime = 0.5f;
	
	// 입력 기록 유효 시간(초). 같은 주기로 타이머가 지난 기록을 지운다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float InputExpirationTime = 0.33f;

private :
	TArray<const FYSCommandSequence*> AllCommandSequence;

	bool bIsInputBlocked;
	TWeakObjectPtr<UYSAbilitySystemComponent> AbilitySystemComponent;
};
