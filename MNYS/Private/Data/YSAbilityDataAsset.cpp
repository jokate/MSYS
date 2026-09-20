// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/YSAbilityDataAsset.h"

#include "YSAbilitySystemComponent.h"
#include "Ability/YSGameplayEffectHandler.h"
#include "Character/YSCharacterBase.h"

UYSAbilityDataAsset* UYSAbilityDataAsset::GetDataAssetFromAbilitySystemComponent(UYSAbilitySystemComponent* InASC)
{
	return InASC->GrantAbilityData;
}

TArray<FYSGrantedAbilityData> UYSAbilityDataAsset::GetAllAbilities() const
{
	TArray<FYSGrantedAbilityData> RetVal;
	
	RetVal.Append(BasicAttackAbilities);
	RetVal.Append(AvoidanceAbilities);
	RetVal.Append(SkillAbilities);
	RetVal.Append(AirAttackAbilities);
	RetVal.Add(DeathAbility);
	RetVal.Add(HitAbility);
	
	return RetVal;
}

#if WITH_EDITOR
void UYSAbilityDataAsset::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
	FYSGameplayEffectHandler::SyncAllIn(GetClass(), this);
}
#endif

UYSTaggedMontageAsset* UYSTaggedMontageAsset::GetMontageAsset(AActor* InActor)
{
	if ( AYSCharacterBase* YSCharacter = Cast<AYSCharacterBase>(InActor) )
	{
		return YSCharacter->TaggedMontageAsset;
	}
	
	return nullptr;
}

namespace
{
	int32 GetTagDepth(const FGameplayTag& Tag)
	{
		return Tag.GetGameplayTagParents().Num();
	}
}

TSoftObjectPtr<UAnimMontage> UYSTaggedMontageAsset::SelectBest(const FGameplayTagContainer& Query) const
{
	const FYSTaggedMontageEntry* Best = nullptr;
	int32 BestScore = -1;

	for (const FYSTaggedMontageEntry& Entry : MontageEntries)
	{
		if ( Entry.Montage.IsNull() )
			continue;
		
		if ( Query.HasAll(Entry.RequiredTags) == false )
			continue;
		
		int32 Score = 0;
		for (const FGameplayTag& RequiredTag : Entry.RequiredTags)
			Score += GetTagDepth(RequiredTag);

		if ( Score > BestScore )
		{
			BestScore = Score;
			Best = &Entry;
		}
	}

	return Best ? Best->Montage : nullptr;
}
