// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTargetActor.h"
#include "YSTA_Trace.generated.h"

struct FGameplayEventData;

UCLASS()
class MNYS_API AYSTA_Trace : public AGameplayAbilityTargetActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AYSTA_Trace();


protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaTime) override;

public:
	virtual void StartTargeting(UGameplayAbility* Ability) override;

	virtual FGameplayAbilityTargetDataHandle MakeTargetData();

	virtual void ConfirmTargetingAndContinue() override;
	
	virtual void OnAnimNotifyStateEnd(const FGameplayEventData* EventData);

	virtual void TraceStart();

	virtual void InitializeData(const FName& InTargetDamageInfo);

	virtual void ProcessHitResult(const TArray<FHitResult>& HitResults);

	virtual void ProcessDamage(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC);

	virtual void ApplyBuff(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> BuffEffectClass );

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MUTA")
	TObjectPtr<USceneComponent> RootCollisionComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MUTA")
	FName WeaponSocketName = NAME_None;

	UPROPERTY()
	TArray<UShapeComponent*> DamageShapes;
	
	UPROPERTY()
	TArray<TWeakObjectPtr<AActor>> QueryActors;

	UPROPERTY()
	FName TargetDamageInfo;
};
