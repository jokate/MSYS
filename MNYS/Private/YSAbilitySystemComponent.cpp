// Fill out your copyright notice in the Description page of Project Settings.


#include "YSAbilitySystemComponent.h"

#include "Ability/YSGameplayAbility.h"
#include "General/YSGeneratedGameplayTags.h"


// Sets default values for this component's properties
UYSAbilitySystemComponent::UYSAbilitySystemComponent()
{

}

void UYSAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UYSAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	// 어빌리티에 대한 등록이 이루어진 경우. 더해주고 빼준다. 차후 해당 데이터를 기반으로 해서 어빌리티를 추적하자.
	AbilitySpecHandles.Emplace(AbilitySpec.Handle);
}

void UYSAbilitySystemComponent::GiveAbilities()
{
	for ( int32 i = 0; i < AbilityClasses.Num(); ++i )
	{
		TSubclassOf<UYSGameplayAbility> AbilityClass = AbilityClasses[i];
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass);
		
		if ( i == 0 )
		{
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(YSTags::IdleInputAttack);	
		}
		
		GiveAbility(AbilitySpec);
	}
}

void UYSAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	AbilitySpecHandles.Remove(AbilitySpec.Handle);
	Super::OnRemoveAbility(AbilitySpec);
}

bool UYSAbilitySystemComponent::ProcessSkillActive(const FGameplayTag& InputTag)
{
	for ( FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitySpecHandles )
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilitySpecHandle);

		if ( AbilitySpec == nullptr )
			continue;

        if ( AbilitySpec->IsActive() )
        	continue;
        
		if ( AbilitySpec->GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			TryActivateAbility(AbilitySpecHandle);
			return true;
		}
	}

	return false;
}

bool UYSAbilitySystemComponent::ProcessAlreadyActiveAbility(const FGameplayTag& InputTag)
{
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
		UYSGameplayAbility* TargetAbil = Cast<UYSGameplayAbility>(AbilitySpec->GetPrimaryInstance());

		if ( IsValid(TargetAbil) == false || TargetAbil->IsActive() == false )
			continue;

		if (TargetAbil->TryTransition(InputTag))
			return true;
	}

	return false;
}


void UYSAbilitySystemComponent::ProcessAbilityByInputPass(const FGameplayTag& InputTag)
{
	if ( ProcessAlreadyActiveAbility(InputTag))
		return;

	ProcessSkillActive(InputTag);
}


