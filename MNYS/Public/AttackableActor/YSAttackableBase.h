// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "General/YSEnum.h"
#include "YSAttackableBase.generated.h"

UCLASS()
class MNYS_API AYSAttackableBase : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AYSAttackableBase();
	virtual void AllocateInstigator(AActor* InInstigator);
protected : 
	UFUNCTION(BlueprintNativeEvent)
	void OnActivate();
	virtual void OnActivate_Implementation() {};
protected : 
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;
	
	// 해당 부분은 차후 활용 여지가 높아서 만들어 놓는다.
	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Root")
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> RootMesh;
};
