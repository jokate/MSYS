// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorValidatorBase.h"
#include "YSAssetValidators.generated.h"

/**
 * 길이 0 이거나 세그먼트가 없는 몽타주를 잡는다.
 * 세그먼트가 애님 0초에서 시작하지 않는 몽타주를 복제하면 이 상태가 된다.
 */
UCLASS()
class UYSMontageValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
};

/**
 * 컴파일된 Playbacks 와 편집용 그래프 노드의 MontageSelector 가 어긋났는지 본다.
 * 같은 값이 두 곳에 저장되므로, 컴파일러를 안 거치고 한쪽만 고치면 어긋난다.
 */
UCLASS()
class UYSPlaybackGraphValidator : public UEditorValidatorBase
{
	GENERATED_BODY()

protected:
	virtual bool CanValidateAsset_Implementation(const FAssetData& InAssetData, UObject* InObject, FDataValidationContext& InContext) const override;
	virtual EDataValidationResult ValidateLoadedAsset_Implementation(const FAssetData& InAssetData, UObject* InAsset, FDataValidationContext& Context) override;
};
