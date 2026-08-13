// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "YSSkillIndicator.generated.h"

class UDecalComponent;
class UMaterialInterface;
class UMaterialInstanceDynamic;

/**
 * 인디케이터가 그릴 도형입니다.
 * 실제 형태는 데칼 머티리얼이 ShapeType 파라미터를 보고 그립니다.
 * 머티리얼에 넘어가는 값이므로 순서를 바꾸면 머티리얼도 같이 고쳐야 합니다.
 */
UENUM(BlueprintType)
enum class EYSIndicatorShapeType : uint8
{
	None	UMETA(DisplayName = "없음"),
	Circle	UMETA(DisplayName = "원형"),
	Cone	UMETA(DisplayName = "부채꼴"),
	Line	UMETA(DisplayName = "직선"),
	Square	UMETA(DisplayName = "사각형")
};

UCLASS(Blueprintable)
class MNYS_API AYSSkillIndicator : public AActor
{
	GENERATED_BODY()

public:
	AYSSkillIndicator();

	/** 로컬 플레이어용 인스턴스를 하나 만든다. 숨겨진 상태로 생성된다. */
	static AYSSkillIndicator* CreateIndicator(const UObject* WorldContextObject, TSubclassOf<AYSSkillIndicator> IndicatorClass);

	// ── 도형 갱신 ────────────────────────────────────────────────────────
	// 전부 매 프레임 호출을 전제로 한다. 도형이 그대로면 파라미터만 덮어쓴다.

	/** 원형 장판. Center는 지면 위 지점. */
	UFUNCTION(BlueprintCallable, Category = "YS | Indicator", meta = (DisplayName = "원형 표시"))
	void ShowCircle(const FVector& Center, float Radius);

	/** 부채꼴. Origin에서 Yaw 방향으로 AngleDegrees 만큼 벌어진다. */
	UFUNCTION(BlueprintCallable, Category = "YS | Indicator", meta = (DisplayName = "부채꼴 표시"))
	void ShowCone(const FVector& Origin, float Yaw, float Radius, float AngleDegrees);

	/** 직선 스킬샷. Origin에서 Yaw 방향으로 Length 만큼 뻗고 폭은 Width. */
	UFUNCTION(BlueprintCallable, Category = "YS | Indicator", meta = (DisplayName = "직선 표시"))
	void ShowLine(const FVector& Origin, float Yaw, float Length, float Width);

	/** 사각형 스킬샷. Center는 지면 위 방향으로 Length 만큼 뻗고 폭은 Width. */
	UFUNCTION(BlueprintCallable, Category = "YS | Indicator", meta = (DisplayName = "사각형 표시"))
	void ShowSquare(const FVector& Center, float Length, float Width);

	/**
	 * 최대 사거리 링. 도형과 별개로 시전자 발밑에 남는다.
	 * @param bShow false면 링만 끈다. 도형은 그대로 유지된다.
	 */
	UFUNCTION(BlueprintCallable, Category = "YS | Indicator", meta = (DisplayName = "사거리 링 표시"))
	void SetRangeRing(bool bShow, const FVector& Center = FVector::ZeroVector, float Range = 0.f);

	/** 사거리 밖·경로 막힘 등으로 시전할 수 없으면 false. 색상이 전환된다. */
	UFUNCTION(BlueprintCallable, Category = "YS | Indicator", meta = (DisplayName = "시전 가능 여부 설정"))
	void SetValid(bool bInValid);

	/** 인디케이터를 숨긴다. 액터는 파괴하지 않는다. */
	UFUNCTION(BlueprintCallable, Category = "YS | Indicator", meta = (DisplayName = "비활성화"))
	void Deactivate();

	UFUNCTION(BlueprintPure, Category = "YS | Indicator")
	bool IsActivated() const { return CurrentShape != EYSIndicatorShapeType::None; }

	UFUNCTION(BlueprintPure, Category = "YS | Indicator")
	EYSIndicatorShapeType GetCurrentShape() const { return CurrentShape; }

protected:
	virtual void BeginPlay() override;

	/**
	 * 도형을 전환하고 액터를 보이게 한다.
	 * @param Origin  도형의 기준점 (원형은 중심, 부채꼴·직선은 시작점)
	 * @param Yaw     도형이 향하는 방향. 원형은 의미 없다.
	 */
	void ApplyShape(EYSIndicatorShapeType NewShape, const FVector& Origin, float Yaw);

	/** 데칼 크기와 기준점으로부터의 전방 오프셋을 함께 적용한다. */
	void SetShapeExtent(float ForwardOffset, float HalfWidth, float HalfLength);

	void EnsureDynamicMaterials();

	void RefreshColor();

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YS | Indicator")
	TObjectPtr<USceneComponent> SceneRoot;

	/** 도형 데칼. 데칼이라 경사면·계단에도 자연스럽게 붙는다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YS | Indicator")
	TObjectPtr<UDecalComponent> ShapeDecal;

	/** 최대 사거리 링. 시전자 발밑에 고정되어 도형과 따로 움직인다. */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "YS | Indicator")
	TObjectPtr<UDecalComponent> RangeDecal;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "YS | Indicator", meta = (DisplayName = "도형 머티리얼"))
	TObjectPtr<UMaterialInterface> ShapeMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Indicator", meta = (DisplayName = "사거리 링 머티리얼"))
	TObjectPtr<UMaterialInterface> RangeMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Indicator", meta = (DisplayName = "시전 가능 색상"))
	FLinearColor ValidColor = FLinearColor(0.10f, 0.60f, 1.00f, 1.f);

	UPROPERTY(EditDefaultsOnly, Category = "YS | Indicator", meta = (DisplayName = "시전 불가 색상"))
	FLinearColor InvalidColor = FLinearColor(1.00f, 0.15f, 0.10f, 1.f);

	/**
	 * 데칼 투영 깊이. 이 깊이만큼 아래로 쏜다.
	 * 경사·계단에서 도형이 끊기면 늘리고, 벽이나 난간에 묻어나면 줄인다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "YS | Indicator", meta = (DisplayName = "투영 깊이", ClampMin = "1.0"))
	float ProjectionDepth = 512.f;

private:
	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> ShapeMID;

	UPROPERTY()
	TObjectPtr<UMaterialInstanceDynamic> RangeMID;

	EYSIndicatorShapeType CurrentShape = EYSIndicatorShapeType::None;

	bool bIsValid = true;
};
