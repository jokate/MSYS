// Fill out your copyright notice in the Description page of Project Settings.


#include "YSAbilitySystemComponent.h"


// Sets default values for this component's properties
UYSAbilitySystemComponent::UYSAbilitySystemComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UYSAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UYSAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	// 어빌리티에 대한 등록이 이루어진 경우. 더해주고 빼준다. 차후 해당 데이터를 기반으로 해서 어빌리티를 추적하자.
	AbilitySpecs.Emplace(AbilitySpec.Handle);
}

void UYSAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	AbilitySpecs.Remove(AbilitySpec.Handle);
	Super::OnRemoveAbility(AbilitySpec);
}

void UYSAbilitySystemComponent::ProcessAbilityByInputPass(const FGameplayTag& InputTag)
{
}


