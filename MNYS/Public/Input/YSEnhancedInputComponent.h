// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EnhancedInputComponent.h"
#include "YSInputConfig.h"
#include "YSEnhancedInputComponent.generated.h"

USTRUCT(BlueprintType)
struct FInputActionWrapper
{
	GENERATED_BODY()
	FInputActionWrapper() { InputAction = nullptr;}
	
	FInputActionWrapper(const UInputAction* InInputAction) {InputAction = InInputAction;}
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	const UInputAction* InputAction;

	bool operator==(const FInputActionWrapper& InputActionWrapper) const
	{
		return InputAction == InputActionWrapper.InputAction;
	}
};

FORCEINLINE uint32 GetTypeHash(const FInputActionWrapper& InInputActionWrapper)
{
	return FCrc::MemCrc32(&InInputActionWrapper, sizeof(FInputActionWrapper));
}

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSEnhancedInputComponent : public UEnhancedInputComponent
{
	GENERATED_BODY()

public:
	/**
	 * 태그로 InputAction을 찾아 바인딩한다.
	 * 페이로드 인자를 가변으로 받으므로 (InputTag, EYSInputPhase)처럼 2개 이상도 넘길 수 있다.
	 */
	template<class UserClass, typename FuncType, typename... VarTypes>
	void BindActionByTag(const UYSInputConfig* InputConfig, const FGameplayTag& InputTag, ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, VarTypes... Vars);

	const FGameplayTag& GetGameplayTagByInputAction(const UInputAction* InputAction);

	virtual void ClearActionBindings() override;
protected :
	
	UPROPERTY(VisibleAnywhere)
	TMap<FInputActionWrapper, FGameplayTag> InputGameplayTagMap;
};


template <class UserClass, typename FuncType, typename... VarTypes>
void UYSEnhancedInputComponent::BindActionByTag(const UYSInputConfig* InputConfig, const FGameplayTag& InputTag,
	ETriggerEvent TriggerEvent, UserClass* Object, FuncType Func, VarTypes... Vars)
{
	if (InputConfig == nullptr)
	{
		return;
	}
	if (const UInputAction* IA = InputConfig->FindInputActionForTag(InputTag))
	{
		BindAction(IA, TriggerEvent, Object, Func, Vars...);
		FInputActionWrapper Wrapper(IA);

		InputGameplayTagMap.Emplace(Wrapper,InputTag);
	}
}