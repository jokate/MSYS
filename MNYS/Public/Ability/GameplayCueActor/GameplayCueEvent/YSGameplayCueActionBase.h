// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "Engine/Scene.h"
#include "GameplayEffectTypes.h"
#include "General/YSEnum.h"
#include "General/YSStruct.h"
#include "Character/Components/YSCameraManageComponent.h"
#include "YSGameplayCueActionBase.generated.h"

class UNiagaraComponent;
class UNiagaraSystem;
class AYSGameplayCueNotifyBase;
class ALevelSequenceActor;
class UPostProcessComponent;
class UMeshComponent;
class UAudioComponent;
class USoundBase;
class UCameraShakeBase;
class APlayerCameraManager;
/**
 * 큐 하나에 얹히는 연출 트랙 1개.
 * 큐가 켜진 뒤 "시작" 에 Begin, "길이" 가 지나거나 큐가 꺼지면 End 가 불린다.
 * 구간이 다른 연출(히트스톱 → 왜곡 → 창 표시)을 큐 하나의 트랙 목록으로 놓기 위한 것이다.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, BlueprintType, Blueprintable)
class MNYS_API UYSGameplayCueActionBase : public UObject
{
	GENERATED_BODY()

public :
	virtual void OnActive(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget, const FGameplayCueParameters& Parameters);
	virtual void OnRemove(AYSGameplayCueNotifyBase* GameplayCueNotify, AActor* MyTarget, const FGameplayCueParameters& Parameters);

	virtual UWorld* GetWorld() const override;

protected :
	virtual void Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters) {}
	virtual void End(AActor* MyTarget, const FGameplayCueParameters& Parameters) {}

	AActor* SelectActor(EYSCueActorSelector Selector, AActor* MyTarget, const FGameplayCueParameters& Parameters) const;

private :
	void HandleBegin();
	void HandleEnd();

protected :
	UPROPERTY(EditAnywhere, Category = "YS | Track", meta = (DisplayName = "사용"))
	bool bEnabled = true;

	UPROPERTY(EditAnywhere, Category = "YS | Track", meta = (DisplayName = "시작 (초)", ClampMin = "0.0", EditCondition = "bEnabled"))
	float StartTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "YS | Track", meta = (DisplayName = "길이 (초, 0 = 큐가 꺼질 때까지)", ClampMin = "0.0", EditCondition = "bEnabled"))
	float Duration = 0.f;

	UPROPERTY()
	TWeakObjectPtr<AYSGameplayCueNotifyBase> OwningGameplayCueNotify;

private :
	TWeakObjectPtr<AActor> ActiveTarget;
	FGameplayCueParameters ActiveParameters;
	FTimerHandle BeginTimerHandle;
	FTimerHandle EndTimerHandle;
	bool bBegun = false;
};

UCLASS(DisplayName = "시퀀스 재생")
class MNYS_API UYSGameplayCueAction_SequencePlay : public UYSGameplayCueActionBase
{
	GENERATED_BODY()

protected:
	virtual void Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void End(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

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

protected :
	virtual void Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void End(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	// 에셋은 Soft로 잡아서 로딩 부담 분리
	UPROPERTY(EditDefaultsOnly, Category = "YS | FX", meta = (DisplayName = "나이아가라 이펙트"))
	TSoftObjectPtr<UNiagaraSystem> NiagaraEffect;

	UPROPERTY(EditDefaultsOnly, Category = "YS | FX", meta = (DisplayName = "붙일 것인가?"))
	bool bNeedToAttach = false;

	UPROPERTY(EditDefaultsOnly, Category = "YS | FX", meta = (DisplayName = "붙일 소켓 이름", EditCondition = "bNeedToAttach"))
	FName AttachSocketName;

	UPROPERTY(EditDefaultsOnly, Category = "YS | FX", meta = (DisplayName = "붙일 오프셋(회전)", EditCondition = "bNeedToAttach"))
	FRotator AttachmentOffset;

	UPROPERTY(EditDefaultsOnly, Category = "YS | FX", meta = (DisplayName = "붙일 오프셋(위치)", EditCondition = "bNeedToAttach"))
	FVector AttachmentLocationOffset;

private :
	TWeakObjectPtr<UNiagaraComponent> SpawnedNiagaraComponent;
};

/**
 * 비율 0 · 전원 · 입력 보관이면 히트스톱이다.
 * 적만 느리게 하려면 범위를 "큐 대상의 적만" 으로 둔다 — 큐 대상(플레이어)은 평속.
 */
UCLASS(DisplayName = "타임 딜레이션 적용")
class MNYS_API UYSGameplayCueAction_TimeDilation : public UYSGameplayCueActionBase
{
	GENERATED_BODY()

protected :
	virtual void Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void End(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | GameplayCue", meta = (DisplayName = "딜레이 비율"))
	float TimeDilation = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | GameplayCue", meta = (DisplayName = "적용 대상"))
	EYSTimeDilationScope Scope = EYSTimeDilationScope::All;

	// 이 트랙 동안 큐 대상의 첫 누름 하나를 보관했다가 트랙이 끝날 때 흘린다 (선입력).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | GameplayCue", meta = (DisplayName = "멈춘 동안 입력 보관"))
	bool bHoldInput = false;
};

UCLASS(DisplayName = "카메라 다이나믹 효과")
class MNYS_API UYSGameplayCueAction_CameraEffect : public UYSGameplayCueActionBase
{
	GENERATED_BODY()

protected:
	virtual void Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void End(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "카메라 효과 파라미터"))
	FYSCameraEffectParams CameraParams;

	// 저스트 회피 큐는 "저스트 회피" 로 둬야 락온 · 조준 카메라 위에 얹힌다.
	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "카메라 우선순위"))
	EYSCameraModePriority CameraPriority = EYSCameraModePriority::Default;

	// 적에게 켜는 큐(브레이크)는 "시전자" 로 둬야 플레이어 카메라가 움직인다.
	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "카메라 주인"))
	EYSCueActorSelector CameraOwner = EYSCueActorSelector::CueTarget;

private:
	// End 때 주인을 다시 찾으면 파괴 · 교대로 컨트롤러가 없을 때 해제를 놓친다.
	TWeakObjectPtr<UYSCameraManageComponent> ActiveCameraManage;
};

UCLASS(DisplayName = "카메라 흔들림")
class MNYS_API UYSGameplayCueAction_CameraShake : public UYSGameplayCueActionBase
{
	GENERATED_BODY()

protected:
	virtual void Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void End(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	// 비어 있으면 아무것도 하지 않는다.
	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "흔들림"))
	TSubclassOf<UCameraShakeBase> Shake;

	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "세기", ClampMin = "0.0"))
	float Scale = 1.f;

	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "카메라 주인"))
	EYSCueActorSelector CameraOwner = EYSCueActorSelector::CueTarget;

	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "트랙이 끝나면 멈춤"))
	bool bStopOnEnd = false;

private:
	TWeakObjectPtr<UCameraShakeBase> ActiveShake;
	TWeakObjectPtr<APlayerCameraManager> ActiveCameraManager;
};

/**
 * 채도 · 비네트 같은 화면 후처리. 큐 대상에 경계 없는 후처리 컴포넌트를 붙였다 뗀다.
 * 쓸 항목만 후처리 값에서 체크한다. 윤곽선을 그리는 머티리얼도 여기 Blendables 에 넣는다.
 */
UCLASS(DisplayName = "화면 효과 (후처리)")
class MNYS_API UYSGameplayCueAction_PostProcess : public UYSGameplayCueActionBase
{
	GENERATED_BODY()

protected:
	virtual void Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void End(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "후처리 값"))
	FPostProcessSettings Settings;

	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "들어오는 시간 (초)", ClampMin = "0.0"))
	float FadeInTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "빠지는 시간 (초)", ClampMin = "0.0"))
	float FadeOutTime = 0.f;

	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "우선순위"))
	float Priority = 10.f;

private:
	void TickFade();
	void DestroyComponent();

	TWeakObjectPtr<UPostProcessComponent> SpawnedComponent;
	FTimerHandle FadeTimerHandle;
	float FadeStartTime = 0.f;
	float FadeStartWeight = 0.f;
	float FadeTargetWeight = 1.f;
	float FadeLength = 0.f;
};

/**
 * 대상 액터의 메시에 커스텀 뎁스 스텐실을 켠다. 실제 선은 스텐실을 읽는 후처리 머티리얼이 그린다.
 * 끝나면 켜기 전 값으로 되돌린다.
 */
UCLASS(DisplayName = "윤곽선 (커스텀 뎁스 스텐실)")
class MNYS_API UYSGameplayCueAction_Outline : public UYSGameplayCueActionBase
{
	GENERATED_BODY()

protected:
	virtual void Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void End(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "대상"))
	EYSCueActorSelector Target = EYSCueActorSelector::CueTarget;

	UPROPERTY(EditAnywhere, Category = "YS | GameplayCue", meta = (DisplayName = "스텐실 값", ClampMin = "0", ClampMax = "255"))
	int32 StencilValue = 1;

private:
	struct FSavedDepthState
	{
		TWeakObjectPtr<UMeshComponent> Mesh;
		bool bRenderCustomDepth = false;
		int32 StencilValue = 0;
	};
	TArray<FSavedDepthState> SavedStates;
};

UCLASS(DisplayName = "사운드")
class MNYS_API UYSGameplayCueAction_Sound : public UYSGameplayCueActionBase
{
	GENERATED_BODY()

protected:
	virtual void Begin(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;
	virtual void End(AActor* MyTarget, const FGameplayCueParameters& Parameters) override;

	// 비어 있으면 아무것도 하지 않는다.
	UPROPERTY(EditAnywhere, Category = "YS | Sound", meta = (DisplayName = "사운드"))
	TSoftObjectPtr<USoundBase> Sound;

	UPROPERTY(EditAnywhere, Category = "YS | Sound", meta = (DisplayName = "2D 로 재생"))
	bool bPlay2D = true;

	UPROPERTY(EditAnywhere, Category = "YS | Sound", meta = (DisplayName = "트랙이 끝나면 멈춤"))
	bool bStopOnEnd = false;

private:
	TWeakObjectPtr<UAudioComponent> SpawnedAudio;
};
