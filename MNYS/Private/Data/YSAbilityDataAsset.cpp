// Fill out your copyright notice in the Description page of Project Settings.


#include "Data/YSAbilityDataAsset.h"

#include "YSAbilitySystemComponent.h"
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
	// 태그의 계층 깊이. Hit → 1, Hit.Big → 2, Direction.Forward → 2.
	// GetGameplayTagParents 는 자기 자신을 포함한 조상 전체를 돌려주므로 그 개수가 곧 깊이다.
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

		// 요구 태그를 하나라도 만족하지 못하면 탈락.
		// HasAll 은 부모 매칭이라 조회에 Hit.Big 만 있어도 요구 Hit 은 만족한다.
		if ( Query.HasAll(Entry.RequiredTags) == false )
			continue;

		// 개수가 아니라 깊이 합으로 점수를 낸다.
		// 개수로 재면 {Hit} 과 {Hit.Big} 이 동점이 되어 배열 순서로 결과가 갈린다.
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
