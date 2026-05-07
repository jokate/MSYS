// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotifyState/YSAnimNotifyState_AddLooseTag.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "General/YSGameplayTag.h"

void UYSAnimNotifyState_AddLooseTag::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                                 float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());

	if ( IsValid(ASC) )
	{
		FGameplayTagContainer LooseTags = CustomTags;
		
		if ( bBlockInput )
			LooseTags.AddTag(YSTags::BlockInput);
		
		if ( bAcceptAbilityInput )
			LooseTags.AddTag(YSTags::AcceptAbilityInput);
		
		ASC->AddLooseGameplayTags(LooseTags);
	}
}

void UYSAnimNotifyState_AddLooseTag::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
	const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(MeshComp->GetOwner());

	if ( IsValid(ASC) )
	{
		FGameplayTagContainer LooseTags = CustomTags;
		
		if ( bBlockInput )
			LooseTags.AddTag(YSTags::BlockInput);
		
		if ( bAcceptAbilityInput )
			LooseTags.AddTag(YSTags::AcceptAbilityInput);
		
		ASC->RemoveLooseGameplayTags(LooseTags);
	}
}
