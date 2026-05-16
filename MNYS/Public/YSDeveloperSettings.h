// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Data/YSDataStruct.h"
#include "Engine/DeveloperSettings.h"
#include "Input/Combo/YSComboData.h"
#include "YSDeveloperSettings.generated.h"

/**
 * 
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "YS Developer Settings"))
class MNYS_API UYSDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public :
	static const FYSCommandSequence* GetCommandSequence(const FName& RowName);
	static const FYSDamageInfo* GetDamageInfo(const FName& RowName);
	
	static TArray<const FYSCommandSequence*> GetAllCommandSequences();
		
private : 
	template <class T>
	static const T* GetRegistryData(const FName& RegistryName, const FName& RowName);
	
	template <class T>
	static TArray<const T*> GetAllRegistryData(const FName& RegistryName);
	
protected :
	UPROPERTY(EditAnywhere, Config, Category = "Data Registry")
	FName CommandRegistryName;

	UPROPERTY(EditAnywhere, Config, Category = "Data Registry")
	FName DamageRegistryName;
	
};
