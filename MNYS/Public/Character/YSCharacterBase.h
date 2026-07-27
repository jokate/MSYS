// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GenericTeamAgentInterface.h"
#include "YSBattleActor.h"
#include "Data/YSDataStruct.h"
#include "GameFramework/Character.h"
#include "YSCharacterBase.generated.h"

class UYSCharacterMovementComponent;
class UYSMotionWarpingComponent;
class UYSAbilitySystemComponent;
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class MNYS_API AYSCharacterBase : public ACharacter, public IAbilitySystemInterface, public IGenericTeamAgentInterface, public IYSBattleActor
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AYSCharacterBase(const FObjectInitializer& ObjectInitializer);
	virtual float GetHpRatio() const override;
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void PostInitializeComponents() override;
	virtual bool IsDead() const override;
public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual UYSCharacterMovementComponent* GetYSCharacterMovement();

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	virtual void SetGenericTeamId(const FGenericTeamId& TeamID) override;
	virtual FGenericTeamId GetGenericTeamId() const override { return GenericTeamId; }
	
	const FYSCharacterInfo* GetCharacterInfo() const;
public :
	UPROPERTY(BlueprintReadWrite, Category = "MotionWarping", EditDefaultsOnly )
	TObjectPtr<UYSMotionWarpingComponent> MotionWarpingComponent;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "AbilitySystemComponent")
	TObjectPtr<UYSAbilitySystemComponent> AbilitySystemComponent;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly, Category = "Weapon")
	TObjectPtr<USkeletalMeshComponent> WeaponMeshComponent;
	
	UPROPERTY(EditDefaultsOnly, meta = (RowType = "/Script/MNYS.YSCharacterInfo"))
	FDataTableRowHandle CharacterInfo;
	
	UPROPERTY()
	FGenericTeamId GenericTeamId;
};
