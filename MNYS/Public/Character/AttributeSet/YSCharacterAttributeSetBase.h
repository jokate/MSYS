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

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FYSOnCharacterDowned, AActor*, DownedActor);

UCLASS()
class MNYS_API UYSCharacterAttributeSetBase : public UAttributeSet
{
	GENERATED_BODY()

public :
	UYSCharacterAttributeSetBase();
	
	virtual void PostInitProperties() override;
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, Hp);
    ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, MaxHp);
	
    ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, MeleeAttackDmg);
    ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, RangedAttackDmg);
    ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, DefenseRate);
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, MoveSpeed);
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, TagGaugeRate);
	
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, RangeAttackCount);
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, MaxRangeAttackCount);
	
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, StatHP);
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, StatMEL);
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, StatRNG);
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, StatAGI);
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, StatSCL);
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, StatSYN);
	ATTRIBUTE_ACCESSORS(UYSCharacterAttributeSetBase, IncomingDamage);
	
 
	/** Health 가 0에 도달했을 때. 스쿼드가 구독해 강제 태그를 건다. */
	FYSOnCharacterDowned OnCharacterDowned;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;
	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	virtual void PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data) override;
	
	void HandleDowned();
	
protected :
	void AutoRegisterHandler();
	
protected :
	
	UPROPERTY(BlueprintReadOnly, Category = "YS | Character", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData Hp;
	UPROPERTY(BlueprintReadOnly, Category = "YS | Character", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MaxHp;
	UPROPERTY(BlueprintReadOnly, Category = "YS | Character", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData MeleeAttackDmg;
	UPROPERTY(BlueprintReadOnly, Category = "YS | Character")
	FGameplayAttributeData RangedAttackDmg;
	UPROPERTY(BlueprintReadOnly, Category = "YS | Character", Meta = (AllowPrivateAccess = true))
	FGameplayAttributeData DefenseRate;
	UPROPERTY(BlueprintReadOnly, Category = "YS | Tag")
	FGameplayAttributeData TagGaugeRate;
	UPROPERTY(BlueprintReadOnly, Category = "YS | Combat")
	FGameplayAttributeData MoveSpeed;
	
	UPROPERTY(BlueprintReadOnly, Category = "YS | Stat")
	FGameplayAttributeData StatHP;
	UPROPERTY(BlueprintReadOnly, Category = "YS | Stat")
	FGameplayAttributeData StatMEL;
	UPROPERTY(BlueprintReadOnly, Category = "YS | Stat")
	FGameplayAttributeData StatRNG;
	UPROPERTY(BlueprintReadOnly, Category = "YS | Stat")
	FGameplayAttributeData StatAGI;
	UPROPERTY(BlueprintReadOnly, Category = "YS | Stat")
	FGameplayAttributeData StatSYN;
	UPROPERTY(BlueprintReadOnly,  Category = "YS | Stat")
	FGameplayAttributeData StatSCL;
	
	UPROPERTY(BlueprintReadOnly, Category = "YS | Range")
	FGameplayAttributeData RangeAttackCount;	
	UPROPERTY(BlueprintReadOnly, Category = "YS | Range")
	FGameplayAttributeData MaxRangeAttackCount;
	
	UPROPERTY(BlueprintReadOnly, Category = "YS | Meta")
	FGameplayAttributeData IncomingDamage;
	
private : 
	using FClampHandler = TFunction<void(float&)>;
	TMap<FGameplayAttribute, FClampHandler> ClampRegistry;
};
