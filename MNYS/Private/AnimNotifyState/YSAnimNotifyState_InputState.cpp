// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState/YSAnimNotifyState_InputState.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "Components/SkeletalMeshComponent.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"

void UYSAnimNotifyState_InputState::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	AActor* OwnerActor = MeshComp->GetOwner();
	const UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(OwnerActor);
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OwnerActor);

	if ( IsValid(InputStateMachine) == false || IsValid(ASC) == false )
	{
		return;
	}

	// 노티파이 객체는 이 몽타주를 쓰는 모든 캐릭터가 같이 쓴다. 핸들을 여기 두지 않고 출처(this)로 찾아 뗀다.
	const FActiveGameplayEffectHandle Handle = InputStateMachine->ApplyStateEffect(InputState, this);

	if ( Handle.IsValid() == false )
	{
		return;
	}

	if ( UYSGameplayAbility* AnimatingAbility = Cast<UYSGameplayAbility>(ASC->GetAnimatingAbility()) )
	{
		AnimatingAbility->AddRuntimeEffectSpecHandle(Handle);
	}
}

void UYSAnimNotifyState_InputState::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                              const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());

	if ( IsValid(ASC) == false )
	{
		return;
	}

	FGameplayEffectQuery Query;
	Query.CustomMatchDelegate.BindLambda([this](const FActiveGameplayEffect& ActiveEffect)
	{
		return ActiveEffect.Spec.GetEffectContext().GetSourceObject() == this;
	});

	ASC->RemoveActiveEffects(Query);
}
