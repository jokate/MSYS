// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemComponent.h"
#include "AttributeSet.h"
#include "General/YSMacros.h"
#include "YSCharacterAttributeSetBase.generated.h"

/**
 * 
 */
UCLASS()
class MNYS_API UYSCharacterAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public :
	UYSCharacterAttributeSetBase();
	
	virtual void PostInitProperties() override;
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, CurrentHp);
    ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, MaxHp);

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

protected :
	void AutoRegisterHandler();
	
protected :
		
	UPROPERTY(BlueprintReadOnly, Category = "YS | Character", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData CurrentHp;
	UPROPERTY(BlueprintReadOnly, Category = "YS | Character", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHp;

private : 
	using FClampHandler = TFunction<void(float&)>;
	TMap<FGameplayAttribute, FClampHandler> ClampRegistry;
};
