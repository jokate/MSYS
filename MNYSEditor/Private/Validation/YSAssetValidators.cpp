// Fill out your copyright notice in the Description page of Project Settings.


#include "Validation/YSAssetValidators.h"

#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Ability/AbilityComponent/YSPlaybackGraphAsset.h"
#include "Animation/AnimMontage.h"
#include "EdGraph/EdGraph.h"
#include "Misc/DataValidation.h"
#include "Playback/YSPlaybackGraphNode.h"

bool UYSMontageValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject != nullptr && InObject->IsA<UAnimMontage>();
}

EDataValidationResult UYSMontageValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	const UAnimMontage* Montage = CastChecked<UAnimMontage>(InAsset);

	int32 SegmentCount = 0;

	for (const FSlotAnimationTrack& SlotTrack : Montage->SlotAnimTracks)
	{
		SegmentCount += SlotTrack.AnimTrack.AnimSegments.Num();
	}

	if (SegmentCount == 0)
	{
		AssetFails(InAsset, FText::FromString(TEXT("몽타주에 애님 세그먼트가 없다.")));
		return EDataValidationResult::Invalid;
	}

	if (Montage->GetPlayLength() <= 0.f)
	{
		AssetFails(InAsset, FText::FromString(TEXT("몽타주 길이가 0 이다. 복제로 깨졌을 수 있다 — 새로 만들고 세그먼트·길이·노티파이를 다시 얹을 것.")));
		return EDataValidationResult::Invalid;
	}

	AssetPasses(InAsset);
	return EDataValidationResult::Valid;
}

bool UYSPlaybackGraphValidator::CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const
{
	return InObject != nullptr && InObject->IsA<UYSPlaybackGraphAsset>();
}

EDataValidationResult UYSPlaybackGraphValidator::ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context)
{
	const UYSPlaybackGraphAsset* Asset = CastChecked<UYSPlaybackGraphAsset>(InAsset);

	if (Asset->EdGraph == nullptr)
	{
		AssetPasses(InAsset);
		return EDataValidationResult::Valid;
	}

	TArray<const UYSAbilityPlaybackBase*> NodePlaybacks;

	for (const UEdGraphNode* Node : Asset->EdGraph->Nodes)
	{
		const UYSPlaybackGraphNode_State* StateNode = Cast<UYSPlaybackGraphNode_State>(Node);

		if (StateNode != nullptr && StateNode->Playback != nullptr)
		{
			NodePlaybacks.Add(StateNode->Playback);
		}
	}

	if (NodePlaybacks.Num() != Asset->Playbacks.Num())
	{
		AssetFails(InAsset, FText::FromString(FString::Printf(
			TEXT("그래프 상태 노드는 %d개인데 컴파일된 Playbacks 는 %d개다. 그래프 에디터에서 다시 컴파일할 것."),
			NodePlaybacks.Num(), Asset->Playbacks.Num())));
		return EDataValidationResult::Invalid;
	}

	// 컴파일러의 정렬 규칙을 여기서 되풀이하지 않으려고 순서 무관으로 짝을 맞춘다.
	EDataValidationResult Result = EDataValidationResult::Valid;

	for (int32 Index = 0; Index < Asset->Playbacks.Num(); ++Index)
	{
		const UYSAbilityPlaybackBase* Compiled = Asset->Playbacks[Index];

		if (Compiled == nullptr)
		{
			AssetFails(InAsset, FText::FromString(FString::Printf(TEXT("Playbacks[%d] 가 null 이다."), Index)));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		const int32 MatchIndex = NodePlaybacks.IndexOfByPredicate([Compiled](const UYSAbilityPlaybackBase* NodePlayback)
		{
			return NodePlayback->MontageSelector == Compiled->MontageSelector;
		});

		if (MatchIndex == INDEX_NONE)
		{
			AssetFails(InAsset, FText::FromString(FString::Printf(
				TEXT("Playbacks[%d] 의 MontageSelector 와 같은 값을 가진 그래프 노드가 없다. 한쪽만 고쳐졌다 — 그래프 노드를 고치고 다시 컴파일할 것."),
				Index)));
			Result = EDataValidationResult::Invalid;
			continue;
		}

		NodePlaybacks.RemoveAtSwap(MatchIndex);
	}

	if (Result == EDataValidationResult::Valid)
	{
		AssetPasses(InAsset);
	}

	return Result;
}
