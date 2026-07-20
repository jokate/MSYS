// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystem/YSWorldTagSubsystem.h"

#include "Engine/World.h"

UYSWorldTagSubsystem* UYSWorldTagSubsystem::Get(const UObject* WorldContextObject)
{
	if (IsValid(WorldContextObject) == false)
	{
		return nullptr;
	}

	const UWorld* World = WorldContextObject->GetWorld();
	return IsValid(World) ? World->GetSubsystem<UYSWorldTagSubsystem>() : nullptr;
}

void UYSWorldTagSubsystem::AddWorldTag(const FGameplayTag& Tag)
{
	if (Tag.IsValid() == false)
	{
		return;
	}

	int32& Count = TagCounts.FindOrAdd(Tag);
	if (++Count == 1)
	{
		WorldTags.AddTag(Tag);
	}
}

void UYSWorldTagSubsystem::RemoveWorldTag(const FGameplayTag& Tag)
{
	int32* Count = TagCounts.Find(Tag);
	if (Count == nullptr)
	{
		return;
	}

	if (--(*Count) <= 0)
	{
		TagCounts.Remove(Tag);
		WorldTags.RemoveTag(Tag);
	}
}

bool UYSWorldTagSubsystem::HasWorldTagMatching(const FGameplayTag& TagToMatch) const
{
	return WorldTags.HasTag(TagToMatch);
}
