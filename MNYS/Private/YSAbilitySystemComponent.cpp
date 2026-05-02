// Fill out your copyright notice in the Description page of Project Settings.


#include "YSAbilitySystemComponent.h"


// Sets default values for this component's properties
UYSAbilitySystemComponent::UYSAbilitySystemComponent()
{

}


// Called when the game starts
void UYSAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	GetWorld()->GetTimerManager().SetTimer(TimerHandle, this, &UYSAbilitySystemComponent::ResetInputTags, InputProcessingTime, false);
}

void UYSAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorld()->GetTimerManager().ClearTimer(TimerHandle);
	Super::EndPlay(EndPlayReason);
}

void UYSAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	// 어빌리티에 대한 등록이 이루어진 경우. 더해주고 빼준다. 차후 해당 데이터를 기반으로 해서 어빌리티를 추적하자.
	AbilitySpecHandles.Emplace(AbilitySpec.Handle);
}

void UYSAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	AbilitySpecHandles.Remove(AbilitySpec.Handle);
	Super::OnRemoveAbility(AbilitySpec);
}


bool UYSAbilitySystemComponent::ProcessSkill(const FGameplayTag& InputTag)
{
	for ( FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitySpecHandles)
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilitySpecHandle);

		if ( AbilitySpec == nullptr )
			continue;

		// 만약 타겟으로 하는 어빌리티 태그가 있는 경우 스킬 즉시 발동.
		if ( AbilitySpec->GetDynamicSpecSourceTags().HasTagExact(InputTag) )
		{
			TryActivateAbility(AbilitySpecHandle);
			return true;
		}
	}

	return false;
}

void UYSAbilitySystemComponent::ProcessCombo(const FGameplayTag& InputTag)
{
	InputTags.Emplace(InputTag);
	for ( FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitySpecHandles )
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilitySpecHandle);

		if ( AbilitySpec == nullptr )
			continue;

		// 미 작동 중인 어빌리티의 경우에는 굳이 처리하지 말자.
		if ( AbilitySpec->IsActive() == false )
		{
			continue;
		}

		// 여기서 이제 추가 처리
		// -> 여기서 알아 처리. -> 여기서 추가 처리는 필요할 듯.
	}
}

void UYSAbilitySystemComponent::ProcessAbilityByInputPass(const FGameplayTag& InputTag)
{
	// 초기 InputTag로 즉발될 스킬이 존재하다면 스킬 공격을 처리하자.
	if ( ProcessSkill(InputTag) )
		return;

	ProcessCombo(InputTag);
}


