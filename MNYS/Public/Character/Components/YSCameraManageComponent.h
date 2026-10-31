// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "General/YSStruct.h"
#include "YSCameraManageComponent.generated.h"


class AYSCharacterPlayer;
class AYSPlayerController;

/** 카메라 모드 우선순위 대역. 같은 값이면 나중에 들어온 요청이 이긴다. 새 대역은 임의 숫자 대신 여기에 항목을 추가한다 (에디터에서 고를 수 있어야 덮는 관계가 한눈에 보인다). */
UENUM(BlueprintType)
enum class EYSCameraModePriority : uint8
{
	Default		= 0		UMETA(DisplayName = "기본 (일회성 연출)"),
	LockOn		= 10	UMETA(DisplayName = "락온"),
	Targeting	= 20	UMETA(DisplayName = "조준 / 스킬 레디"),
	Impact		= 30	UMETA(DisplayName = "타격 임팩트 줌"),
	JustAvoid	= 50	UMETA(DisplayName = "저스트 회피"),
	Break		= 60	UMETA(DisplayName = "브레이크 진입"),
	Cinematic	= 100	UMETA(DisplayName = "시네마틱 (연계기)"),
};

/** 카메라 배치 요청 1건. Requester 는 약참조라 요청자가 사라지면 자동 정리된다 (EndAbility 를 못 거친 어빌리티 대비). */
USTRUCT()
struct FYSCameraModeRequest
{
	GENERATED_BODY()

	UPROPERTY()
	TWeakObjectPtr<UObject> Requester;

	UPROPERTY()
	FYSCameraEffectParams Params;

	UPROPERTY()
	EYSCameraModePriority Priority = EYSCameraModePriority::Default;
};
	

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class MNYS_API UYSCameraManageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UYSCameraManageComponent();
	static UYSCameraManageComponent* Get(const AActor* Character);

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

protected :
	void ChaseCamera(AActor* LockedTarget, float DeltaTime);

public :
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "잠금 시 카메라 회전 보간 속도"))
	float CameraInterpSpeed = 0.3f;

	UPROPERTY()
	TWeakObjectPtr<AYSPlayerController> OwnerPlayerController;

	UPROPERTY()
	TWeakObjectPtr<AYSCharacterPlayer> OwnerPlayer;

public:
	/** 카메라 배치를 요청한다. 같은 Requester 가 다시 부르면 갱신한다. Priority 는 EYSCameraModePriority 대역. */
	void PushCameraMode(UObject* Requester, const FYSCameraEffectParams& Params, EYSCameraModePriority Priority);

	/** 요청을 회수한다. 남은 요청이 있으면 그쪽으로, 없으면 기본값으로 되돌아간다. */
	void PopCameraMode(UObject* Requester);

	bool HasCameraMode(const UObject* Requester) const;

	// 기존 호출부 호환용. 내부적으로 Push/Pop 으로 내려간다.
	void StartCameraEffect(UObject* Requester, const FYSCameraEffectParams& Params);
	void StopCameraEffect(UObject* Requester);

	/** 임팩트 줌. 활성 모드(임팩트 줌 제외, 없으면 기본 카메라)에 행의 거리 · 높이 · 화각 증감을 얹어 ZoomDuration 초 뒤 뺀다. 다시 부르면 갱신. */
	void PlayImpactZoom(const FYSImpactProfile& Profile);

private:
	/** 스위치 창이 열린 순간(타겟 바꿈 · 태그 교대)에 표의 임팩트를 건다. */
	UFUNCTION()
	void HandleSwitchWindowOpened(EYSSwitchTrigger Trigger, AActor* Target, float AbuseCooldown);

	/** 폰이 바뀌면 캐시와 기본값을 다시 잡는다. 스쿼드 태그 교대 대응. */
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

	/** 락온 대상이 바뀌면 몸을 돌리고 락온 카메라 모드를 건다. null 이면 모드를 뺀다. */
	UFUNCTION()
	void HandleLockOnChanged(AActor* LockedTarget);

	AActor* GetLockedTarget() const;

	/** 현재 폰의 카메라 기본값을 복귀 지점으로 캡처한다. */
	void CaptureCameraDefaults();

	/** 우선순위가 가장 높은 유효 요청. 죽은 요청자는 이 과정에서 정리된다. */
	const FYSCameraModeRequest* ResolveActiveMode();

	void TickCameraEffect(float DeltaTime);

public :
	bool IsControlPitchLocked() const { return bControlPitchLockRequested; }
	bool IsControlYawLocked() const { return bControlYawLockRequested; }

protected :

	void TickControlPitch(const FYSCameraEffectParams* Params, float DeltaTime, float Speed);

	/** 커서가 화면 중앙에서 벗어난 만큼 스프링암의 주시점을 민다. */
	void TickCursorLean(const FYSCameraEffectParams* Params, float DeltaTime);

	/** 커서를 화면 중앙 기준 -1~1 로 정규화한다. 데드존을 뺀 뒤 단위원으로 클램프한다. */
	bool GetNormalizedCursorOffset(float DeadZone, FVector2D& OutOffset) const;

private:
	// 우선순위로 정렬하지 않는다. 요청 수가 한 자릿수라 매 틱 선형 탐색이 더 싸다.
	UPROPERTY()
	TArray<FYSCameraModeRequest> CameraModeStack;

	/** 락온이 스택에 밀어넣을 프로파일. 끄면 락온은 회전만 담당한다. */
	UPROPERTY(EditDefaultsOnly, Category = "YS | Camera", meta = (DisplayName = "락온 시 카메라 모드 사용"))
	bool bUseLockOnCameraMode = false;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Camera",
		meta = (DisplayName = "락온 카메라 프로파일", EditCondition = "bUseLockOnCameraMode", EditConditionHides))
	FYSCameraEffectParams LockOnCameraParams;

	/** 스택이 빈 뒤 기본값으로 되돌아갈 때 쓸 속도. 마지막으로 빠진 요청의 값을 물려받는다. */
	float RestoreInterpSpeed = 5.f;

	// 스택이 요청자를 약참조로만 들어서 임팩트 줌의 요청자는 여기서 붙든다.
	UPROPERTY()
	TObjectPtr<UObject> ImpactZoomRequester;

	FTimerHandle ImpactZoomTimer;

	/** 스택이 비고 기본값에 도달하면 참. 매 틱 보간을 건너뛴다. */
	bool bCameraSettled = true;

	float DefaultArmLength    = 600.f;
	FVector DefaultSocketOffset = FVector::ZeroVector;
	float DefaultFOV          = 90.f;
	FRotator DefaultRotation = FRotator::ZeroRotator;
	bool bOriginControl = false;
	bool bControlYaw = false;
	
	float PreOverrideControlPitch = 0.f;

	/** 지금 피치를 잠그고 있거나 복귀 보간 중인가. 내부 상태 머신용. */
	bool bIsControlPitchOverridden = false;
	
	bool bControlPitchLockRequested = false;

	/** "도달 후 피치 입력 허용" 모드에서 목표 각도에 닿았는가. 닿은 뒤에는 피치를 플레이어에게 돌려준다. */
	bool bControlPitchReached = false;

	bool bControlYawLockRequested = false;
};
