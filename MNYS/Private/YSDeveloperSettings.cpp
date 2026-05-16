// Fill out your copyright notice in the Description page of Project Settings.


#include "YSDeveloperSettings.h"

#include "DataRegistryId.h"
#include "DataRegistrySubsystem.h"

template <class T>
const T* UYSDeveloperSettings::GetRegistryData(const FName& RegistryName, const FName& RowName)
{
	FDataRegistryId RegistryId;
	RegistryId.RegistryType = RegistryName;
	RegistryId.ItemName = RowName;

	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	
	if ( IsValid(DataRegistrySubsystem) == false )
	{
		return nullptr;
	}

	return DataRegistrySubsystem->GetCachedItem<T>(RegistryId);
}

const FYSCommandSequence* UYSDeveloperSettings::GetComboData(const FName& RowName)
{
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	if (IsValid(Settings) == false || Settings->ComboRegistryName.IsNone())
	{
		return nullptr;
	}

	return GetRegistryData<FYSCommandSequence>(Settings->ComboRegistryName, RowName);
}

const FYSDamageInfo* UYSDeveloperSettings::GetDamageInfo(const FName& RowName)
{
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	if (IsValid(Settings) == false || Settings->DamageRegsitryName.IsNone())
	{
		return nullptr;
	}

	return GetRegistryData<FYSDamageInfo>(Settings->DamageRegsitryName, RowName);
}
