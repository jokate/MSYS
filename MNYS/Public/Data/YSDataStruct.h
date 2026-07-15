

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "General/YSEnum.h"
#include "YSDataStruct.generated.h"

/**
 * 프로젝트 규칙 : 해당 프로젝트는 DataRegistry를 기반으로 합니다.
 */

class UNiagaraSystem;

UENUM(BlueprintType)
enum class EYSDamageType : uint8
{
	None,
	Physical,
	Magical,
	TrueDamage,
};

// Purpose : 데미지 처리를 위한, 데이터 ( 어빌리티에 대한 데미지 정보를 근간으로 합니다. )
USTRUCT()
struct FYSDamageInfo : public FTableRowBase
{
	GENERATED_BODY()
	
public :
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float AdditiveTrueDamage = 0.f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	EYSDamageType DamageType = EYSDamageType::None;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FGameplayTag HitTag;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraSystem> HitEffect;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<UCameraShakeBase> HitCameraShake;
};


USTRUCT()
struct FYSSkillInfo : public FTableRowBase
{
	GENERATED_BODY()
	
public : 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Cooldown")
	bool bHasCooldown = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float Cooldown = 0.f;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "YS | Type")
	EYSSkillType SkillType = EYSSkillType::Basic;
};