// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "General/YSStruct.h"
#include "YSAbilityDataAsset.generated.h"

class UGameplayEffect;
class UYSAbilitySystemComponent;
/**
 * 
 */
UCLASS()
class MNYS_API UYSAbilityDataAsset : public UDataAsset
{
	GENERATED_BODY()
	
public : 
	static UYSAbilityDataAsset* GetDataAssetFromAbilitySystemComponent(UYSAbilitySystemComponent* InASC);
	TArray<FYSGrantedAbilityData> GetAllAbilities() const;
	TSubclassOf<UGameplayEffect> GetBackupPassiveEffect() const { return BackupPassiveEffect; }
	TSubclassOf<UGameplayEffect> GetStatInitEffect() const { return StatInitEffect; }
	
protected : 
	UPROPERTY(EditDefaultsOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "기본 공격"))
	TArray<FYSGrantedAbilityData> BasicAttackAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "회피"))
	TArray<FYSGrantedAbilityData> AvoidanceAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "스킬"))
	TArray<FYSGrantedAbilityData> SkillAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "공중 공격"))
	TArray<FYSGrantedAbilityData> AirAttackAbilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "죽음"))
	FYSGrantedAbilityData DeathAbility;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "히트"))
	FYSGrantedAbilityData HitAbility;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "백업 패시브	"))
	TSubclassOf<UGameplayEffect> BackupPassiveEffect;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | GameplayAbility", meta = (DisplayName = "스탯 GE"))
	TSubclassOf<UGameplayEffect> StatInitEffect;
};

USTRUCT(BlueprintType)
struct FYSTaggedMontageEntry
{
    GENERATED_BODY()

    // 이 몽타주가 뽑히기 위해 조회 태그가 전부 만족해야 하는 조건
    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "필요 태그"))
    FGameplayTagContainer RequiredTags;

    UPROPERTY(EditDefaultsOnly, meta = (DisplayName = "몽타주"))
    TSoftObjectPtr<UAnimMontage> Montage;
};


UCLASS()
class MNYS_API UYSTaggedMontageAsset : public UDataAsset
{
	GENERATED_BODY()
	
public : 
	static UYSTaggedMontageAsset* GetMontageAsset(AActor* InActor);
	TSoftObjectPtr<UAnimMontage> SelectBest(const FGameplayTagContainer& Query) const;
	
public : 
	UPROPERTY(EditDefaultsOnly, Category = "YS | Montage", meta = (DisplayName = "태그"))
	TArray<FYSTaggedMontageEntry> MontageEntries;
};