// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "YSTransformPolicy.generated.h"

/**
 * 
 */

struct MNYS_API FYSTransformPolicyContext
{
	FYSTransformPolicyContext() = default;
	explicit FYSTransformPolicyContext(AActor* InOwner, AActor* InTarget = nullptr, const FHitResult* InHitResult = nullptr)
		: OwnerActor(InOwner), TargetActor(InTarget), HitResult(InHitResult) {}

	AActor* OwnerActor = nullptr;
	AActor* TargetActor = nullptr;
	const FHitResult* HitResult = nullptr;
};

USTRUCT(BlueprintType, DisplayName = "기본 위치")
struct MNYS_API FYSLocationPolicyBase
{
	GENERATED_BODY()
	virtual ~FYSLocationPolicyBase() = default;
	virtual FVector GetLocation(const FYSTransformPolicyContext& Context) const
	{
		return Context.OwnerActor->GetActorLocation();
	}
	
};

USTRUCT(DisplayName = "소켓 위치")
struct MNYS_API FYSLocationPolicy_Socket : public FYSLocationPolicyBase
{
	GENERATED_BODY()
	
	virtual FVector GetLocation(const FYSTransformPolicyContext& Context) const override;
	
protected : 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket", meta = (DisplayName = "소켓 이름"))
	FName SocketName;
};


USTRUCT(DisplayName = "상대 오프셋")
struct MNYS_API FYSLocationPolicy_RelativeOffset : public FYSLocationPolicyBase
{
	GENERATED_BODY()
	
	virtual FVector GetLocation(const FYSTransformPolicyContext& Context) const override;

protected :
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset", meta = (DisplayName = "상대 오프셋"))
	FVector RelativeOffset;
};

USTRUCT(DisplayName = "랜덤 위치")
struct MNYS_API FYSLocationPolicy_RandomOffset : public FYSLocationPolicyBase
{
	GENERATED_BODY()
	
	virtual FVector GetLocation(const FYSTransformPolicyContext& Context) const override;
	
protected : 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Random", meta = (DisplayName = "랜덤 범위"))
	FVector RandomOffsetRange;
};

USTRUCT(DisplayName = "타겟팅 위치")
struct MNYS_API FYSLocationPolicy_Targeting : public FYSLocationPolicyBase
{
	GENERATED_BODY()
	
	virtual FVector GetLocation(const FYSTransformPolicyContext& Context) const override;
};

USTRUCT(DisplayName = "히트된 위치")
struct MNYS_API FYSLocationPolicy_HitResult : public FYSLocationPolicyBase
{
	GENERATED_BODY()
	
	virtual FVector GetLocation(const FYSTransformPolicyContext& Context) const override;
};


USTRUCT(BlueprintType, DisplayName = "기본 회전")
struct MNYS_API FYSRotationPolicyBase
{
	GENERATED_BODY()
	virtual ~FYSRotationPolicyBase() = default;

	virtual FRotator GetRotation(const FYSTransformPolicyContext& Context) const
	{
		return Context.OwnerActor->GetActorRotation();
	}
};

USTRUCT(DisplayName = "소켓 회전")
struct MNYS_API FYSRotationPolicy_Socket : public FYSRotationPolicyBase
{
	GENERATED_BODY()
	
	virtual FRotator GetRotation(const FYSTransformPolicyContext& Context) const override;
	
public : 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Socket", meta = (DisplayName = "소켓 이름"))
	FName SocketName;
};

USTRUCT(DisplayName = "컨트롤 로테이션")
struct MNYS_API FYSRotationPolicy_Control : public FYSRotationPolicyBase
{
	GENERATED_BODY()
	
	virtual FRotator GetRotation(const FYSTransformPolicyContext& Context) const override;
};

USTRUCT(DisplayName = "락온 타겟 회전")
struct MNYS_API FYSRotationPolicy_LockOnTarget : public FYSRotationPolicyBase
{
	GENERATED_BODY()
	
	virtual FRotator GetRotation(const FYSTransformPolicyContext& Context) const override;
};

USTRUCT(DisplayName = "플레이백 타겟 회전")
struct MNYS_API FYSRotationPolicy_PlaybackTarget : public FYSRotationPolicyBase
{
	GENERATED_BODY()

	virtual FRotator GetRotation(const FYSTransformPolicyContext& Context) const override;
};

USTRUCT(DisplayName = "상대 오프셋")
struct MNYS_API FYSRotationPolicy_RelativeOffset : public FYSRotationPolicyBase
{
	GENERATED_BODY()
	
	virtual FRotator GetRotation(const FYSTransformPolicyContext& Context) const override;
	
protected : 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Offset", meta = (DisplayName = "상대 오프셋"))
	FRotator RelativeOffset;
};

USTRUCT(DisplayName = "타겟팅 회전")
struct MNYS_API FYSRotationPolicy_Targeting : public FYSRotationPolicyBase
{
	GENERATED_BODY()
	
	virtual FRotator GetRotation(const FYSTransformPolicyContext& Context) const override;
};


USTRUCT(BlueprintType, DisplayName = "트랜스폼 정책")
struct MNYS_API FYSTransformPolicy
{
	GENERATED_BODY()
	
public : 
	FTransform GetFinalTransform(const FYSTransformPolicyContext& Context) const;
	
public : 
	UPROPERTY(EditAnywhere, meta = (DisplayName = "로케이션 정책", BaseStruct = "/Script/MNYS.YSLocationPolicy"))
	FInstancedStruct LocationPolicy;
	
	UPROPERTY(EditAnywhere, meta = (DisplayName = "로테이션 정책", BaseStruct = "/Script/MNYS.YSRotationPolicy"))
	FInstancedStruct RotationPolicy;	
};
