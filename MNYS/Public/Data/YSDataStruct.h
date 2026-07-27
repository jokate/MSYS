

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "General/YSEnum.h"
#include "YSDataStruct.generated.h"

/**
 * 프로젝트 규칙 : 해당 프로젝트는 DataRegistry를 기반으로 합니다.
 */

class UNiagaraSystem;
class UGameplayEffect;

UENUM(BlueprintType)
enum class EYSDamageType : uint8
{
	None,
	Range,
	Melee,
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

USTRUCT(BlueprintType)
struct FYSStatBlock
{
	GENERATED_BODY()
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS|Stat", meta = (ClampMin = "1", ClampMax = "10"))
	int32 HP = 5;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS|Stat", meta = (ClampMin = "1", ClampMax = "10"))
	int32 MEL = 5;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS|Stat", meta = (ClampMin = "1", ClampMax = "10"))
	int32 RNG = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS|Stat", meta = (ClampMin = "1", ClampMax = "10"))
	int32 AGI = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS|Stat", meta = (ClampMin = "1", ClampMax = "10"))
	int32 SYN = 5;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS|Stat", meta = (ClampMin = "1", ClampMax = "10"))
	int32 SCL = 5;
};

USTRUCT()
struct FYSCharacterInfo : public FTableRowBase
{
	GENERATED_BODY()
	
public : 
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Identity")
	FGameplayTag CharacterTag;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Identity")
	FText DisplayName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Identity")
	TObjectPtr<UTexture2D> Portrait;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Identity")
	TSubclassOf<APawn> PawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Stat")
	FYSStatBlock Stats;
	
	// 1단계 : 1~10 스케일의 6스탯을 SetByCaller 로 StatXXX 어트리뷰트에 그대로 꽂는다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Kit", meta = (DisplayName = "스탯 주입 이펙트 (SetByCaller)"))
	TSubclassOf<UGameplayEffect> StatInitEffect;

	// 2단계 : StatXXX 를 AttributeBased 로 읽어 MaxHp/공격력/이동속도 등 실전투 수치로 환산한다.
	// 1단계와 분리한 이유는, 각성 제단/봉인 해방으로 StatXXX 가 런 도중 변할 때
	// 이 이펙트만 재적용하면 파생 수치가 다시 산출되도록 재사용 지점을 만들기 위함이다.
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "YS | Kit", meta = (DisplayName = "스탯 환산 이펙트 (AttributeBased)"))
	TSubclassOf<UGameplayEffect> StatDeriveEffect;
};