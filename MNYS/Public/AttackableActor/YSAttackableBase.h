// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "GameFramework/Actor.h"
#include "General/YSEnum.h"
#include "Interface/YSDamageProxy.h"
#include "Interface/YSSpawnInitializable.h"
#include "YSAttackableBase.generated.h"

class AYSTelegraphActor;
class UAbilitySystemComponent;

// 위치 기준은 직접 스폰한 액터, 공격자는 스폰 체인의 루트.
// 프록시로 공격자를 넘겨서, 이 액터가 무엇을 스폰하든 공격자가 체인을 따라 전달된다.
UCLASS()
class MNYS_API AYSAttackableBase : public AActor, public IYSSpawnInitializable, public IYSDamageProxy
{
	GENERATED_BODY()

public:
	AYSAttackableBase();
	virtual void AllocateInstigator(AActor* InInstigator);

	virtual AActor* GetDamageInstigator() const override { return InstigatorActor.Get(); }

	virtual bool OnSpawnInitialize(AActor* InOwnerActor, AActor* InInstigator, const TSharedPtr<FYSAbilityHitContext>& InHitContext) override;
	
	void InitializeHitContext(const TSharedPtr<FYSAbilityHitContext>& InHitContext)
	{
		HitContext = InHitContext;
	}
	
	virtual void SetPoolActive(bool bActive) override;

	/** 지연 스폰은 액터가 나오기 전이라 클래스 기본 객체로 부른다. */
	virtual AYSTelegraphActor* SpawnTelegraph(UWorld* World, const FTransform& Transform, float Duration) const { return nullptr; }

protected :
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	void _NotifyLinkedActorEnded();
	UAbilitySystemComponent* _FindEventSourceASC() const;

	UFUNCTION(BlueprintNativeEvent)
	void OnActivate();
	virtual void OnActivate_Implementation();
	
	void ProcessActivationType();	
	void DeprocessActivationType();
	
	void OnActivateTagCallback(const FGameplayEventData* GameplayEventData);
	void ProcessTelegraph();
	void ClearTelegraph();

	/** 0 이하면 바로 풀로 돌아간다. */
	void ReturnToPoolAfter(float Delay);
protected : 
	// 스포너보다 여기서 처리하는 편이 범위 등 더 많은 정보를 얻을 수 있어 다시 돌린다.
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
	
	UPROPERTY()
	TWeakObjectPtr<AActor> InstigatorActor;
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

	// 낸 어빌리티의 배율. 스폰할 때 받아 둔다.
	float EffectScale = 1.f;
private :
	FTimerHandle ActivateTimerHandle;
	FTimerHandle DestroyTimerHandle;
	FDelegateHandle TagEventHandle;

	UPROPERTY()
	TWeakObjectPtr<UAbilitySystemComponent> TagEventASC;
};
