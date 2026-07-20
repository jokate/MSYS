// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Subsystems/WorldSubsystem.h"
#include "YSWorldTagSubsystem.generated.h"

UCLASS()
class MNYS_API UYSWorldTagSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public :
	static UYSWorldTagSubsystem* Get(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "YS | WorldTag", meta = (DisplayName = "월드 태그 추가 (중첩 카운트)"))
	void AddWorldTag(const FGameplayTag& Tag);

	UFUNCTION(BlueprintCallable, Category = "YS | WorldTag", meta = (DisplayName = "월드 태그 제거 (카운트 0이면 해제)"))
	void RemoveWorldTag(const FGameplayTag& Tag);

	UFUNCTION(BlueprintPure, Category = "YS | WorldTag", meta = (DisplayName = "월드 태그 매칭 확인 (자식 태그가 부모 쿼리에 매칭)"))
	bool HasWorldTagMatching(const FGameplayTag& TagToMatch) const;

private :
	// FGameplayTag는 UObject 참조가 없어 GC 대상이 아니므로 UPROPERTY 없이 보관한다
	TMap<FGameplayTag, int32> TagCounts;
	FGameplayTagContainer WorldTags;
};
