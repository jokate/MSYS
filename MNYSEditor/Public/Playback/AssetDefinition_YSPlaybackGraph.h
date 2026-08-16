// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "AssetDefinitionDefault.h"
#include "AssetDefinition_YSPlaybackGraph.generated.h"

/**
 * 콘텐츠 브라우저에서 이 에셋이 어떻게 보이고 어떻게 열리는지.
 *
 * 별도 등록 코드가 필요 없다 — UAssetDefinition 은 CDO 생성 시점에
 * 스스로 레지스트리에 등록한다. (Editor/AssetDefinition/Private/AssetDefinition.cpp, PostCDOContruct)
 */
UCLASS()
class UAssetDefinition_YSPlaybackGraph : public UAssetDefinitionDefault
{
	GENERATED_BODY()

protected:
	virtual FText GetAssetDisplayName() const override;
	virtual FLinearColor GetAssetColor() const override;
	virtual TSoftClassPtr<UObject> GetAssetClass() const override;
	virtual TConstArrayView<FAssetCategoryPath> GetAssetCategories() const override;
	virtual EAssetCommandResult OpenAssets(const FAssetOpenArgs& OpenArgs) const override;
};
