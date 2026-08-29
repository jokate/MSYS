// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "General/YSStruct.h"
#include "Input/Combo/YSComboData.h"
#include "UObject/Object.h"
#include "YSAbilityEventAction.generated.h"

class AYSSaveEcho;
struct FYSSavedTechnique;
class UYSAbilityTriggerPayload_Velocity;
class UGameplayEffect;
struct FYSComboTransition;
struct FGameplayEventData;
class UYSGameplayAbility;
class UGameplayAbility;
/**
 * 어빌리티 이벤트 어셉팅에 따른, 동작 방식에 대해서 정의합니다.
 * 특정 이벤트가 들어온 상황이라면, 각 상황에 맞게 처리가 될 것입니다.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable)
class MNYS_API UYSAbilityEventAction : public UObject
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintNativeEvent)
	bool Execute(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData);
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) { return true; }
};

// 트레이스 시작.
UCLASS(DisplayName = "트레이스 시작")
class MNYS_API UYSAbilityEventAction_StartTrace : public UYSAbilityEventAction
{
	GENERATED_BODY()

public :
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
};

UCLASS(DisplayName = "트레이스 종료")
class MNYS_API UYSAbilityEventAction_StopTrace : public UYSAbilityEventAction
{
	GENERATED_BODY()

public :
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;	
};

UCLASS(DisplayName = "파괴")
class MNYS_API UYSAbilityEventAction_Destroy : public UYSAbilityEventAction
{
	GENERATED_BODY()
public :
	void DestroyActor();
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;	
	
		
protected :
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "죽은 이후 Destroy 시간"))
	float DestroyTime = 2.0f;
	
	FTimerHandle DeathTimerHandle;

private : 
	UPROPERTY()
	TWeakObjectPtr<AActor> DestroyTarget;
};

// 정지 되었다는 것을 의미.
UCLASS(DisplayName = "(플레이어) State 제거.")
class MNYS_API UYSAbilityEventAction_TransitionState : public UYSAbilityEventAction
{
	GENERATED_BODY()
	
public : 
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
};

UCLASS(DisplayName = "Gameplay Cue 트리거")
class MNYS_API UYSAbilityEventAction_GameplayCue : public UYSAbilityEventAction
{
	GENERATED_BODY()
	
public :
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
	
public : 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "트리거 할 게임 플레이 큐"))
	FGameplayTagContainer GameplayCueTag;
};

UCLASS(DisplayName = "Gameplay Effect 트리거")
class MNYS_API UYSAbilityEventAction_GameplayEffect : public UYSAbilityEventAction
{
	GENERATED_BODY()
	
public : 
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
	
protected : 
	virtual bool Execute_GameplayEffectToSelf(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData);
	virtual bool Execute_GameplayEffectFromInstigator(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData);
	
public :
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "트리거 할 게임 플레이 이펙트"))
	TSubclassOf<UGameplayEffect> GameplayEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "Instigator로부터 받을 것인가?"))
	bool bIsFromInstigator = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "끝났을 때 제거 여부"))
	bool bRemoveWhenFinished = false;
};

UCLASS(DisplayName = "속도 적용")
class MNYS_API UYSAbilityEventAction_ApplyVelocity : public UYSAbilityEventAction
{
	GENERATED_BODY()

public :
	UFUNCTION()
	void OnTimedOut();
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
	
protected :
	UPROPERTY()
	const UYSAbilityTriggerPayload_Velocity* VelocityData = nullptr;
	
	UPROPERTY()
	AActor* OwningActor = nullptr;
};

UCLASS(DisplayName = "액터 스폰")
class MNYS_API UYSAbilityEventAction_SpawnActor : public UYSAbilityEventAction
{
	GENERATED_BODY()

public :
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
};

UCLASS(DisplayName = "컨텍스트 태그 체크")
class MNYS_API UYSAbilityEventAction_CheckContextTag : public UYSAbilityEventAction
{
	GENERATED_BODY()
	
public : 
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
};

UCLASS(Abstract, DisplayName = "EQS 쿼리 실행 / 상속 필요")
class MNYS_API UYSAbilityEventAction_RunEQS : public UYSAbilityEventAction
{
	GENERATED_BODY()
	
public :

	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
	
	UFUNCTION(BlueprintNativeEvent)
	void OnEQSQueryFinished(UYSGameplayAbility* OwningAbility, FVector Location);
	virtual void OnEQSQueryFinished_Implementation(UYSGameplayAbility* OwningAbility, FVector Location);
};

UCLASS(DisplayName = "카메라 진입")
class MNYS_API UYSAbilityEventAction_PushCamera : public UYSAbilityEventAction
{
	GENERATED_BODY()	
	
public : 
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
	
};

UCLASS(DisplayName = "카메라 해제")
class MNYS_API UYSAbilityEventAction_PopCamera : public UYSAbilityEventAction
{
	GENERATED_BODY()	
	
public : 
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
};

UCLASS(DisplayName = "인디케이터 조준")
class MNYS_API UYSAbilityEventAction_AimStart: public UYSAbilityEventAction
{
	GENERATED_BODY()
	
public : 
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
};

UCLASS(DisplayName = "인디케이터 조준 해제")
class MNYS_API UYSAbilityEventAction_AimStop : public UYSAbilityEventAction
{
	GENERATED_BODY()
	
public :
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;	
};

UCLASS(DisplayName = "자원 소모")
class MNYS_API UYSAbilityEventAction_ConsumeResource : public UYSAbilityEventAction
{
	GENERATED_BODY()
	
public : 
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;
	
public : 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "사용할 자원"))
	FGameplayAttribute Attribute;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "자원 소모량"))
	float Amount;
};

UCLASS(DisplayName = "세이브 요청 (혼)")
class MNYS_API UYSAbilityEventAction_SaveAbilityRecord : public UYSAbilityEventAction
{
	GENERATED_BODY()
	
public : 
	virtual bool Execute_Implementation(UYSGameplayAbility* OwningAbility, const FGameplayEventData& EventData) override;

protected :
	bool Execute_TripleEcho(UYSGameplayAbility* OwningAbility, AActor* AvatarActor, const FYSSavedTechnique& Technique);
};