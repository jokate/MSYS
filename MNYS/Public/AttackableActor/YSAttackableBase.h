// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "General/YSEnum.h"
#include "Interface/YSSpawnInitializable.h"
#include "YSAttackableBase.generated.h"

class AYSTelegraphActor;

UCLASS()
class MNYS_API AYSAttackableBase : public AActor, public IYSSpawnInitializable
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AYSAttackableBase();
	virtual void AllocateInstigator(AActor* InInstigator);
	
	virtual void OnSpawnInitialize(AActor* InOwnerActor, const TSharedPtr<FYSAbilityHitContext>& InHitContext) override;
	
	void InitializeHitContext(const TSharedPtr<FYSAbilityHitContext>& InHitContext)
	{
		HitContext = InHitContext;
	}
protected : 
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnActivate();
	virtual void OnActivate_Implementation();
	
	void ProcessActivationType();	
	void DeprocessActivationType();
	
	void OnActivateTagCallback(const FGameplayEventData* GameplayEventData);
	virtual void ProcessTelegraph() {}
	void DestroyActor();
protected : 
	//2026.06.15 다시 돌리는 이유는 스포너에서 처리하는 것보다 차라리 여기서 처리하는게 맞다고 생각이 드는 중.. (범위나 다양한 정보를 얻을 수 있기 때문임)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "ActivationType")
	EYSAttackActivationType ActivationType = EYSAttackActivationType::Instant;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | ActiveTime", meta = (DisplayName = "활성 시간", EditCondition = "ActivationType==EYSAttackActivationType::TimeBased", EditConditionHides))
	float ActivateTime = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | ActiveTime")
	float DestroyDelay = 0.f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | ActiveCondition", meta = (DisplayName = "태그 이벤트", EditCondition = "ActivationType==EYSAttackActivationType::TagBased", EditConditionHides))
	FGameplayTag EventTag;
	
	UPROPERTY()
	TWeakObjectPtr<AActor> OwnerActor;
	
	// 해당 부분은 차후 활용 여지가 높아서 만들어 놓는다.
	UPROPERTY()
	TWeakObjectPtr<AActor> TargetActor;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Root")
	TObjectPtr<USceneComponent> SceneRoot;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Mesh")
	TObjectPtr<UStaticMeshComponent> RootMesh;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Telegraph")
	TSubclassOf<AYSTelegraphActor> TelegraphClass;
	
	UPROPERTY()
	TObjectPtr<AYSTelegraphActor> TelegraphActor;	
	
protected : 
	TSharedPtr<FYSAbilityHitContext> HitContext;
private : 
	FTimerHandle ActivateTimerHandle;
	FTimerHandle DestroyTimerHandle;
};
