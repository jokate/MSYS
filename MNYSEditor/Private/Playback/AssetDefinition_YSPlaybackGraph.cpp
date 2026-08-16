// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/AssetDefinition_YSPlaybackGraph.h"

#include "Ability/AbilityComponent/YSPlaybackGraphAsset.h"
#include "Playback/YSPlaybackGraphEditor.h"

#define LOCTEXT_NAMESPACE "AssetDefinition_YSPlaybackGraph"

FText UAssetDefinition_YSPlaybackGraph::GetAssetDisplayName() const
{
	return LOCTEXT("AssetDisplayName", "플레이백 그래프");
}

FLinearColor UAssetDefinition_YSPlaybackGraph::GetAssetColor() const
{
	return FLinearColor(FColor(90, 160, 255));
}

TSoftClassPtr<UObject> UAssetDefinition_YSPlaybackGraph::GetAssetClass() const
{
	return UYSPlaybackGraphAsset::StaticClass();
}

TConstArrayView<FAssetCategoryPath> UAssetDefinition_YSPlaybackGraph::GetAssetCategories() const
{
	static const FAssetCategoryPath Categories[] = { EAssetCategoryPaths::Gameplay };
	return Categories;
}

EAssetCommandResult UAssetDefinition_YSPlaybackGraph::OpenAssets(const FAssetOpenArgs& OpenArgs) const
{
	const EToolkitMode::Type Mode = OpenArgs.GetToolkitMode();

	for (UYSPlaybackGraphAsset* Asset : OpenArgs.LoadObjects<UYSPlaybackGraphAsset>())
	{
		const TSharedRef<FYSPlaybackGraphEditor> Editor = MakeShared<FYSPlaybackGraphEditor>();
		Editor->InitEditor(Mode, OpenArgs.ToolkitHost, Asset);
	}

	return EAssetCommandResult::Handled;
}

#undef LOCTEXT_NAMESPACE
