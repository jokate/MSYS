// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "YSTargetingShape.generated.h"

class AYSSkillIndicator;

/** 매 틱 산출되는 조준 결과입니다. 확정 시점에 어빌리티가 이 값을 소비합니다. */
USTRUCT(BlueprintType)
struct FYSTargetingResult
{
	GENERATED_BODY()

	/** 도형의 기준점. 조준점 기준 도형은 커서 위치, 시전자 기준 도형은 시전자 위치가 들어온다. */
	UPROPERTY(BlueprintReadOnly, Category = "YS | Targeting")
	FVector Location = FVector::ZeroVector;

	/** 시전자에서 조준점을 향하는 수평 방향. 직선·부채꼴은 이 값만 쓴다. */
	UPROPERTY(BlueprintReadOnly, Category = "YS | Targeting")
	FVector Direction = FVector::ForwardVector;

	/** 대상 지정 도형에서 잡힌 액터. 그 외 도형에서는 항상 null 이다. */
	UPROPERTY(BlueprintReadOnly, Category = "YS | Targeting")
	TObjectPtr<AActor> TargetingActor = nullptr;

	/** 사거리 밖·조준 실패 등으로 시전할 수 없으면 false. */
	UPROPERTY(BlueprintReadOnly, Category = "YS | Targeting")
	bool bValid = false;
};

/**
 * 스킬 하나가 어떤 도형으로 조준되는지 선언합니다.
 *
 * 도형마다 기준점이 다릅니다 —
 *  - 조준점 기준 : 원형·사각형 (커서가 도형의 중심)
 *  - 시전자 기준 : 직선·부채꼴·자기중심원 (커서는 방향만 결정)
 * 이 차이 때문에 사거리 클램프와 사거리 링의 의미도 도형마다 달라집니다.
 */
USTRUCT()
struct FYSTargetingShape
{
	GENERATED_BODY()

	virtual ~FYSTargetingShape() = default;

	/** 사거리 링 표시 여부. 직선처럼 도형 길이가 곧 사거리인 경우 꺼둔다. */
	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "사거리 링 표시"))
	bool bShowRangeRing = true;

	/**
	 * 사거리 밖을 겨눴을 때 끝으로 당길지 여부 (LoL 기본 동작).
	 * 끄면 사거리 밖은 무효 처리되어 인디케이터가 붉게 뜨고 확정이 막힌다.
	 */
	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "사거리 밖이면 끝으로 클램프"))
	bool bClampToRange = true;

	/** 조준점을 도형의 기준점으로 쓰는가. false면 시전자가 기준이고 조준은 방향만 준다. */
	virtual bool UsesAimPointAsOrigin() const { return true; }

	/** 시전자로부터의 최대 사거리. 0이면 사거리 제한이 없다. */
	virtual float GetRange() const { return 0.f; }

	/** 인디케이터에 도형을 그린다. 사거리 링은 여기서 공통 처리한다. */
	void Draw(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const;

protected:
	virtual void DrawShape(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const {}
};


USTRUCT(DisplayName = "원형 (조준점 중심)")
struct FYSTargetingShape_Circle : public FYSTargetingShape
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "최대 사거리"))
	float Range = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "반경"))
	float Radius = 200.f;

	virtual float GetRange() const override { return Range; }

protected:
	virtual void DrawShape(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const override;
};


USTRUCT(DisplayName = "사각형 (조준점 중심)")
struct FYSTargetingShape_Square : public FYSTargetingShape
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "최대 사거리"))
	float Range = 1000.f;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "길이"))
	float Length = 400.f;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "폭"))
	float Width = 400.f;

	virtual float GetRange() const override { return Range; }

protected:
	virtual void DrawShape(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const override;
};


/** 자기 중심 원. 시전자를 중심으로 터지는 스킬. 조준점을 쓰지 않는다. */
USTRUCT(DisplayName = "자기 중심 원")
struct FYSTargetingShape_SelfCircle : public FYSTargetingShape
{
	GENERATED_BODY()

	FYSTargetingShape_SelfCircle()
	{
		// 시전자가 곧 중심이라 사거리 링이 도형 자체와 겹친다. 기본값을 꺼둔다.
		bShowRangeRing = false;
		bClampToRange  = false;
	}

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "반경"))
	float Radius = 400.f;

	virtual bool UsesAimPointAsOrigin() const override { return false; }

protected:
	virtual void DrawShape(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const override;
};


USTRUCT(DisplayName = "직선 (스킬샷)")
struct FYSTargetingShape_Line : public FYSTargetingShape
{
	GENERATED_BODY()

	FYSTargetingShape_Line()
	{
		// 직선은 길이가 곧 사거리다. 링을 겹쳐 그리면 정보가 중복된다.
		bShowRangeRing = false;
		bClampToRange  = false;
	}

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "길이"))
	float Length = 1200.f;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "폭"))
	float Width = 120.f;

	virtual bool UsesAimPointAsOrigin() const override { return false; }

protected:
	virtual void DrawShape(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const override;
};


USTRUCT(DisplayName = "부채꼴")
struct FYSTargetingShape_Cone : public FYSTargetingShape
{
	GENERATED_BODY()

	FYSTargetingShape_Cone()
	{
		bShowRangeRing = false;
		bClampToRange  = false;
	}

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "반경"))
	float Radius = 500.f;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Targeting", meta = (DisplayName = "각도", ClampMin = "1.0", ClampMax = "180.0"))
	float AngleDegrees = 60.f;

	virtual bool UsesAimPointAsOrigin() const override { return false; }

protected:
	virtual void DrawShape(AYSSkillIndicator* Indicator, const FYSTargetingResult& Result, const FVector& CasterLocation) const override;
};
