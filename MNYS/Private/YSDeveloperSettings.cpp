// Fill out your copyright notice in the Description page of Project Settings.


#include "YSDeveloperSettings.h"

#include "DataRegistry.h"
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

template <class T>
TArray<const T*> UYSDeveloperSettings::GetAllRegistryData(const FName& RegistryName)
{
	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	
	if ( IsValid(DataRegistrySubsystem) == false )
	{
		return TArray<const T*>();
	}
	
	UDataRegistry* Registry = DataRegistrySubsystem->GetRegistryForType(RegistryName);
	
	if ( IsValid(Registry) == false )
		return TArray<const T*>();
	
	TArray<const T*> AllRegistry;
	Registry->GetAllItems<T>(TEXT("Preload"), AllRegistry);

	return AllRegistry;
}

const FYSCommandSequence* UYSDeveloperSettings::GetCommandSequence(const FName& RowName)
{
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	if (IsValid(Settings) == false || Settings->CommandRegistryName.IsNone())
	{
		return nullptr;
	}

	return GetRegistryData<FYSCommandSequence>(Settings->CommandRegistryName, RowName);
}

const FYSDamageInfo* UYSDeveloperSettings::GetDamageInfo(const FName& RowName)
{
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	if (IsValid(Settings) == false || Settings->DamageRegistryName.IsNone())
	{
		return nullptr;
	}

	return GetRegistryData<FYSDamageInfo>(Settings->DamageRegistryName, RowName);
}

TArray<const FYSCommandSequence*> UYSDeveloperSettings::GetAllCommandSequences()
{
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	if (IsValid(Settings) == false || Settings->CommandRegistryName.IsNone())
	{
		return TArray<const FYSCommandSequence*>();
	}
	
	return GetAllRegistryData<FYSCommandSequence>(Settings->CommandRegistryName);
}

TArray<FName> UYSDeveloperSettings::GetDamageRowOptions()
{
	TArray<FName> RowNames;

	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	if (IsValid(Settings) == false || Settings->DamageRegistryName.IsNone())
	{
		return RowNames;
	}

	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (IsValid(DataRegistrySubsystem) == false)
	{
		return RowNames;
	}

	UDataRegistry* Registry = DataRegistrySubsystem->GetRegistryForType(Settings->DamageRegistryName);
	if (IsValid(Registry) == false)
	{
		return RowNames;
	}

	// 아이템 캐싱(Acquire) 없이 소스(DataTable 등)에서 가능한 ID를 그대로 열거 → 에디터에서도 안전.
	TArray<FDataRegistryId> RegistryIds;
	Registry->GetPossibleRegistryIds(RegistryIds);

	RowNames.Reserve(RegistryIds.Num());
	for (const FDataRegistryId& RegistryId : RegistryIds)
	{
		RowNames.Add(RegistryId.ItemName);
	}

	return RowNames;
}
