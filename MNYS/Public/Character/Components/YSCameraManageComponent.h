// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "General/YSStruct.h"
#include "YSCameraManageComponent.generated.h"


class AYSCharacterPlayer;
class AYSPlayerController;

/**
 * 카메라 모드 우선순위 대역입니다.
 * 같은 값이면 나중에 들어온 요청이 이깁니다.
 *
 * 값 사이를 비워둔 이유는 나중에 대역을 끼워 넣기 위해서입니다.
 * 새 대역이 필요하면 임의의 숫자를 쓰지 말고 여기에 항목을 추가하십시오 —
 * 에디터에서 고를 수 있어야 어떤 요청이 어떤 요청을 덮는지 한눈에 보입니다.
 */
UENUM(BlueprintType)
enum class EYSCameraModePriority : uint8
{
	Default		= 0		UMETA(DisplayName = "기본 (일회성 연출)"),
	LockOn		= 10	UMETA(DisplayName = "락온"),
	Targeting	= 20	UMETA(DisplayName = "조준 / 스킬 레디"),
	JustAvoid	= 50	UMETA(DisplayName = "저스트 회피"),
	Cinematic	= 100	UMETA(DisplayName = "시네마틱 (연계기)"),
};

/**
 * 카메라 배치 요청 1건입니다.
 * Requester는 약참조라 요청자가 사라지면 자동으로 정리됩니다 —
 * 어빌리티가 EndAbility를 거치지 못하고 파괴돼도 카메라가 물리지 않습니다.
 */
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
	// Sets default values for this component's properties
	UYSCameraManageComponent();
	static UYSCameraManageComponent* Get(const AActor* Character);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;
	virtual void ForceSetLockOn(AActor* TargetToLockOn);
	virtual void ReleaseLockOn();

	virtual AActor* GetCurrentTarget() { return CurrentLockedTarget.Get(); }
protected :
	void ProcessLockOnFunction(float DeltaTime);
	void ChaseCamera(float DeltaTime);

public :
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (DisplayName = "현재 잠금된 대상"))
	TWeakObjectPtr<AActor> CurrentLockedTarget;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (DisplayName = "잠금 대상이 될 수 있는 최대 각도"))
	float CameraInterpSpeed = 0.3f;

	UPROPERTY()
	TWeakObjectPtr<AYSPlayerController> OwnerPlayerController;

	UPROPERTY()
	TWeakObjectPtr<AYSCharacterPlayer> OwnerPlayer;

public:
	/**
	 * 카메라 배치를 요청한다. 같은 Requester가 다시 부르면 기존 요청을 갱신한다.
	 * @param Requester  요청 주체. Pop 할 때 이 포인터로 찾는다.
	 * @param Priority   EYSCameraModePriority 대역 참조.
	 */
	void PushCameraMode(UObject* Requester, const FYSCameraEffectParams& Params, EYSCameraModePriority Priority);

	/** 요청을 회수한다. 남은 요청이 있으면 그쪽으로, 없으면 기본값으로 되돌아간다. */
	void PopCameraMode(UObject* Requester);

	bool HasCameraMode(const UObject* Requester) const;

	// 기존 호출부 호환용. 내부적으로 Push/Pop 으로 내려간다.
	void StartCameraEffect(UObject* Requester, const FYSCameraEffectParams& Params);
	void StopCameraEffect(UObject* Requester);

private:
	/** 폰이 바뀌면 캐시와 기본값을 다시 잡는다. 스쿼드 태그 교대 대응. */
	UFUNCTION()
	void HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn);

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

	bool bControlYawLockRequested = false;
};
