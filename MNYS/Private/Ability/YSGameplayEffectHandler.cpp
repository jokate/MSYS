// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/YSGameplayEffectHandler.h"

#include "GameplayEffect.h"
#include "StructUtils/InstancedStruct.h"

void FYSGameplayEffectHandler::CollectSetByCallerTags(const UGameplayEffect* Effect, TArray<FGameplayTag>& OutTags) const
{
	if (Effect == nullptr)
	{
		return;
	}

	AddIfSetByCaller(Effect->DurationMagnitude, OutTags);
	
	for (const FGameplayModifierInfo& ModifierInfo : Effect->Modifiers)
	{
		AddIfSetByCaller(ModifierInfo.ModifierMagnitude, OutTags);
	}
	
	for (const FGameplayEffectExecutionDefinition& ExecutionDef : Effect->Executions)
	{
		for (const FGameplayEffectExecutionScopedModifierInfo& ModifierInfo : ExecutionDef.CalculationModifiers)
		{
			AddIfSetByCaller(ModifierInfo.ModifierMagnitude, OutTags);
		}
	}
}

bool FYSGameplayEffectHandler::SyncFromEffect()
{
	TArray<FGameplayTag> Required;
	CollectSetByCallerTags(EffectClass ? EffectClass->GetDefaultObject<UGameplayEffect>() : nullptr, Required);

	TMap<FGameplayTag, float> Synced;
	for (const FGameplayTag& Tag : Required)
	{
		Synced.Add(Tag, Magnitudes.FindRef(Tag));
	}

	const bool bChanged = !Synced.OrderIndependentCompareEqual(Magnitudes);
	Magnitudes = MoveTemp(Synced);
	return bChanged;
}

bool FYSGameplayEffectHandler::IsInSync() const
{
	TArray<FGameplayTag> Required;
	CollectSetByCallerTags(EffectClass ? EffectClass->GetDefaultObject<UGameplayEffect>() : nullptr, Required);

	if (Required.Num() != Magnitudes.Num())
	{
		return false;
	}

	for (const FGameplayTag& Tag : Required)
	{
		if (!Magnitudes.Contains(Tag))
		{
			return false;
		}
	}
	
	return true;
}

bool FYSGameplayEffectHandler::SyncAllIn(const UStruct* Type, void* Container)
{
	bool bChanged = false;

	for (FPropertyValueIterator It(FStructProperty::StaticClass(), Type, Container); It; ++It)
	{
		const FStructProperty* Property = CastField<FStructProperty>(It.Key());
		void* Value = const_cast<void*>(It.Value());

		if (Property->Struct == FYSGameplayEffectHandler::StaticStruct())
		{
			bChanged |= static_cast<FYSGameplayEffectHandler*>(Value)->SyncFromEffect();
		}
		else if (Property->Struct == FInstancedStruct::StaticStruct())
		{
			FInstancedStruct* Instanced = static_cast<FInstancedStruct*>(Value);
			if (Instanced->IsValid())
			{
				bChanged |= SyncAllIn(Instanced->GetScriptStruct(), Instanced->GetMutableMemory());
			}
		}
	}

	return bChanged;
}

void FYSGameplayEffectHandler::AddIfSetByCaller(const FGameplayEffectModifierMagnitude& ModifierMagnitude,
                                                TArray<FGameplayTag>& OutTag) const
{
	if ( ModifierMagnitude.GetMagnitudeCalculationType() != EGameplayEffectMagnitudeCalculation::SetByCaller )
	{
		return;
	}
	
	const FGameplayTag& Tag = ModifierMagnitude.GetSetByCallerFloat().DataTag;
	
	if ( Tag.IsValid() )
	{
		OutTag.Add(Tag);
	}
}
