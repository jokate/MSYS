// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "General/YSStruct.h"
#include "YSGameplayCueActionBase.generated.h"

class UNiagaraSystem;
class AYSGameplayCueNotifyBase;
class ALevelSequenceActor;
struct FGameplayCueParameters;
/**
 * 
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable)
class MNYS_API UYSGameplayCueActionBase : public UObject
{
	GENERATED_BODY()
	
public : 
	virtual void OnActive(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget, const FGameplayCueParameters& Parameters);
	virtual void OnRemove(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget, const FGameplayCueParameters& Parameters) { OwningGameplayCueNotify = nullptr; }

protected : 
	UPROPERTY()
	TWeakObjectPtr<AYSGameplayCueNotifyBase> OwningGameplayCueNotify;
};

UCLASS(DisplayName = "시퀀스 재생")
class MNYS_API UYSGameplayCueAction_SequencePlay : public UYSGameplayCueActionBase
{
	GENERATED_BODY()
	
public : 
	virtual void OnActive(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void OnRemove(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | GameplayCue", meta = (DisplayName = "재생할 레벨 시퀀스 액터"))
	TWeakObjectPtr<ALevelSequenceActor> SequenceToPlay;
private:
	UFUNCTION()
	void OnSequenceFinished();
	void StopSequenceSafely();
};

UCLASS(DisplayName = "나이아가라 이펙트 트리거")
class MNYS_API UYSGameplayCueAction_NiagaraEffect : public UYSGameplayCueActionBase
{
	GENERATED_BODY()

public : 
	virtual void OnActive(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	
protected : 
	// 에셋은 Soft로 잡아서 로딩 부담 분리
	UPROPERTY(EditDefaultsOnly, Category = "YS | FX", meta = (DisplayName = "나이아가라 이펙트"))
	TSoftObjectPtr<UNiagaraSystem> NiagaraEffect;
};

UCLASS(DisplayName = "타임 딜레이션 적용")
class MNYS_API UYSGameplayCueAction_TimeDilation : public UYSGameplayCueActionBase
{
	GENERATED_BODY()

public :
	virtual void OnActive(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void OnRemove(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected :
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | GameplayCue", meta = (DisplayName = "딜레이 비율"))
	float TimeDilation = 1.0f;
};

UCLASS(DisplayName = "카메라 다이나믹 효과")
class MNYS_API UYSGameplayCueAction_CameraEffect : public UYSGameplayCueActionBase
{
	GENERATED_BODY()

public:
	virtual void OnActive(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void OnRemove(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

protected:
	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "카메라 효과 파라미터"))
	FYSCameraEffectParams CameraParams;
};