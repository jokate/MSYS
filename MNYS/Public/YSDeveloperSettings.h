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
	static const FYSCommandSequence* GetComboData(const FName& RowName);
	static const FYSDamageInfo* GetDamageInfo(const FName& RowName);
		
private : 
	template <class T>
	static const T* GetRegistryData(const FName& RegistryName, const FName& RowName);
protected :
	UPROPERTY(EditAnywhere, Config, Category = "Data Registry")
	FName ComboRegistryName;

	UPROPERTY(EditAnywhere, Config, Category = "Data Registry")
	FName DamageRegsitryName;
	
};
