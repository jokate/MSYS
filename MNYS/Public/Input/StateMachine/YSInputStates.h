// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataTable.h"
#include "General/YSEnum.h"
#include "UObject/Object.h"
#include "YSInputStates.generated.h"

class UGameplayEffect;
class UYSInputStateMachineComponent;
class UYSAbilitySystemComponent;
class AYSCharacterBase;

/** 창(저스트 회피 · 스위치)처럼 GE 로 잠깐 얹히는 입력 상태의 입력 규칙. */
USTRUCT()
struct FYSInputWindowRule
{
	GENERATED_BODY()

	// 누르면 창을 닫는 입력 (상태 접두 없는 원본). 어빌리티 라우팅이 끝난 뒤 닫는다.
	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "창을 닫는 입력", Categories = "Input"))
	TArray<FGameplayTag> CloseInputs;

	// 조준(Input.Aim)을 누르고 있는 동안 창이 아니라 조준 상태로 해석할 입력.
	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "조준 중엔 조준 상태로 넘길 입력", Categories = "Input"))
	TArray<FGameplayTag> AimHeldPassThroughInputs;

	// 창 상태에서 아무 어빌리티도 받지 않았을 때 창 아래 상태로 다시 해석할 입력. 창은 그 뒤에 닫는다.
	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "못 받으면 아래 상태로 넘길 입력", Categories = "Input"))
	TArray<FGameplayTag> UnhandledFallThroughInputs;

	// 창이 겹치면 값이 큰 창이 위에 남는다. 낮은 창은 그 아래에 있다가 높은 창이 닫히면 드러난다. 같으면 나중에 열린 창이 위.
	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "겹칠 때 우선순위"))
	int32 Priority = 0;
};

/** 입력 상태 정의 표의 행. 행 이름이 곧 상태 이름이고, 입력 태그 앞머리("행이름.Input.X")가 된다. */
USTRUCT(BlueprintType)
struct MNYS_API FYSInputStateDefinition : public FTableRowBase
{
	GENERATED_BODY()

	// 세부 상태 다음 순위로 태그 해석에 참여한다. 먼저 맞는 하나가 채택되므로 좁은 것부터 넣는다.
	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "분류"))
	TArray<FString> CategoryNames = { TEXT("Grounded"), TEXT("Actionable"), TEXT("Alive") };

	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "창"))
	bool bIsWindow = false;

	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "창 규칙", EditCondition = "bIsWindow"))
	FYSInputWindowRule WindowRule;

	// 이 상태를 거는 GE. 입력 상태 GE 컴포넌트의 상태가 이 행 이름이어야 한다. 바닥 상태는 비운다.
	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "이 상태의 GE"))
	TSubclassOf<UGameplayEffect> StateEffect;
};

/** 외부에서 변경 제어되는 StateMachine. 오로지 Input 제어가 목표다. */
UCLASS()
class MNYS_API UYSInputStates : public UObject
{
	GENERATED_BODY()

public :
	virtual void ProcessInput(const FGameplayTag& InputGameplayTag, EYSInputPhase InputPhase);
	void InitState(AActor* Owner, FName InStateName, const FYSInputStateDefinition& InDefinition);
	FName GetStateName() const { return StateName; }

public :
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;

	UPROPERTY()
	TWeakObjectPtr<UYSAbilitySystemComponent> OwnerASC;

	UPROPERTY()
	TWeakObjectPtr<UYSInputStateMachineComponent> OwnerStateMachineManager;

protected :
	/** "StateName.InputTag" 와 "Category.InputTag" 후보를 세부 → 대분류 순으로 만든다. 문자열은 최초 1회만 만들고 이후엔 캐시를 탄다. */
	void ResolveStateTags(const FGameplayTag& InputGameplayTag, TArray<FGameplayTag>& OutTags);

	/** 이 상태로 바꾼 GE(UYSInputStateGEComponent)를 걷어 창을 닫는다. */
	void CloseWindow() const;

	/** 요청 스택에서 이 상태 바로 아래 상태. 맨 아래거나 스택에 없으면 nullptr. */
	UYSInputStates* FindStateBelow(const UYSInputStateMachineComponent* InputStateMachineComponent) const;

	const FYSInputWindowRule* GetWindowRule() const { return Definition.bIsWindow ? &Definition.WindowRule : nullptr; }

protected :
	UPROPERTY()
	FName StateName;

	UPROPERTY()
	FYSInputStateDefinition Definition;

private :
	// InputTag → 후보 태그 목록 캐시. 빈 결과(미등록 조합)도 캐시해 재조회를 막는다.
	TMap<FGameplayTag, TArray<FGameplayTag>> ResolvedTagCache;
};
