// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSCharacterBase.h"
#include "General/YSEnum.h"
#include "YSMonster.generated.h"

class UYSPhaseChangeComponent;
class UYSGroggyComponent;
class UYSGroggyProfile;

UCLASS()
class MNYS_API AYSMonster : public AYSCharacterBase
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AYSMonster(const FObjectInitializer& ObjectInitializer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// 그로기 게이지는 경직 등급 2 적에게만 둔다 (기획 02_COMBAT §5.2).
	virtual void PostInitializeComponents() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
public : 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | PhaseChange")
	TObjectPtr<UYSPhaseChangeComponent> PhaseChangeComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Combat", meta = (DisplayName = "경직 등급"))
	EYSStaggerGrade StaggerGrade = EYSStaggerGrade::Soft;

	// 경직 등급 2 에서만 쓴다. 비우면 YS Developer Settings 의 기본 그로기 규칙.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Combat", meta = (DisplayName = "그로기 프로필"))
	TObjectPtr<UYSGroggyProfile> GroggyProfile;

	// 경직 등급 2 일 때만 PostInitializeComponents 에서 만든다.
	UPROPERTY(Transient, BlueprintReadOnly, Category = "YS | Combat")
	TObjectPtr<UYSGroggyComponent> GroggyComponent;
};
