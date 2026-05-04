// Fill out your copyright notice in the Description page of Project Settings.


#include "YSDeveloperSettings.h"

#include "DataRegistryId.h"
#include "DataRegistrySubsystem.h"

template <class T>
const T* UYSDeveloperSettings::GetRegistryData(FName RegistryName, FName RowName)
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

const FYSComboSequence* UYSDeveloperSettings::GetComboData(FName RowName)
{
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	if (Settings == nullptr || Settings->ComboRegistryName.IsNone())
	{
		return nullptr;
	}

	return GetRegistryData<FYSComboSequence>(Settings->ComboRegistryName, RowName);
}