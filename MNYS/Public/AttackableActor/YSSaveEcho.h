// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayAbilitySpecHandle.h"
#include "Character/Components/YSSaveComponent.h"
#include "GameFramework/Character.h"
#include "Interface/YSDamageProxy.h"
#include "Interface/YSSpawnInitializable.h"
#include "YSSaveEcho.generated.h"


/*
 * 혼의 세이브를 발현할 객체.
 */

struct FAbilityEndedData;
struct FYSSavedTechnique;
class UYSAbilitySystemComponent;

UCLASS()
class MNYS_API AYSSaveEcho : public ACharacter, public IAbilitySystemInterface, public IYSDamageProxy, public IYSSpawnInitializable
{
	GENERATED_BODY()

public :
	AYSSaveEcho();

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	/** 피해·팀 판정의 주체는 언제나 혼이다. */
	virtual AActor* GetDamageInstigator() const override;

	/**
	 * 소환 직후 1회. 주인의 겉모습을 베끼고 저장된 동작을 재생한다.
	 * 재생이 끝나면 스스로 사라진다.
	 */
	void Initialize(ACharacter* InMaster, const FYSSavedTechnique& InTechnique);

	virtual void SetPoolActive(bool bActive) override;

protected :

	UFUNCTION()
	void OnReplayEnded(const FAbilityEndedData& EndedData);

	/** 주인의 메시·애님BP를 그대로 가져온다. 분신은 늘 지금의 혼과 같아 보여야 한다. */
	void MirrorAppearance(const ACharacter* InMaster);
	virtual bool OnSpawnInitialize(AActor* InOwnerActor, AActor* InInstigator, const TSharedPtr<FYSAbilityHitContext>& HitContext) override;

	void ReturnToPoolActor();
protected :
	UPROPERTY(VisibleAnywhere, Category = "YS | Echo", meta = (DisplayName = "어빌리티 시스템"))
	TObjectPtr<UYSAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Echo", meta = (DisplayName = "분신 오버레이 머티리얼"))
	TObjectPtr<UMaterialInterface> EchoOverlayMaterial;

	UPROPERTY(VisibleAnywhere, Category = "YS | Echo", meta = (DisplayName = "이 분신의 주인"))
	TWeakObjectPtr<ACharacter> Master;

	/** 재현이 어떤 이유로든 끝나지 않을 때의 안전망. 분신이 필드에 남는 것을 막는다. */
	UPROPERTY(EditDefaultsOnly, Category = "YS | Echo", meta = (DisplayName = "최대 생존 시간"))
	float MaxLifeTime = 6.f;

private :
	FGameplayAbilitySpecHandle ReplayHandle;
	FTimerHandle LifeTimerHandle;
	
	FYSSavedTechnique SavedTechnique;
};