// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState/YSAnimNotifyState_AddLooseTag.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "General/YSGameplayTag.h"

void UYSAnimNotifyState_AddLooseTag::PostLoad()
{
	Super::PostLoad();

	if ( bBlockInput )
	{
		CustomTags.AddTag(YSTags::BlockInput);
		bBlockInput = false;
	}

	if ( bAcceptAbilityInput )
	{
		CustomTags.AddTag(YSTags::AcceptAbilityInput);
		bAcceptAbilityInput = false;
	}
}

void UYSAnimNotifyState_AddLooseTag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                 float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if ( UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()) )
	{
		ASC->AddLooseGameplayTags(CustomTags);
	}
}

void UYSAnimNotifyState_AddLooseTag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if ( UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner()) )
	{
		ASC->RemoveLooseGameplayTags(CustomTags);
	}
}
