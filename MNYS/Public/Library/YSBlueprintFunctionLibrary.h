// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSEnum.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "YSBlueprintFunctionLibrary.generated.h"

class UYSGameplayAbility;
class UAbilitySystemComponent;
class UYSCharacterAttributeSetBase;

/**
 *
 */
UCLASS()
class MNYS_API UYSBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public :
	UFUNCTION(BlueprintPure)
	static float GetFinalDamage(const UYSCharacterAttributeSetBase* Owner, const UYSCharacterAttributeSetBase* Target, const FName& SkillID);

	UFUNCTION(BlueprintCallable)
	static void SendHitEventToTarget(AActor* Instigator, AActor* Target, const FName& SkillID);

	/**
	 * 공통 히트 검증 파이프라인.
	 * - 아군 필터링, 사망 체크, JustAvoid / Invincible 처리, 데미지 이벤트 전송
	 * - 검증을 통과한 유효 히트는 OnValidated 콜백으로 전달
	 * @param Instigator               공격 발생 주체
	 * @param RawHits                  원본 히트 결과
	 * @param DamageRow                데미지 테이블 Row 이름
	 * @param OnValidated              유효 히트 결과 콜백 (빈 배열이면 호출되지 않음)
	 * @param bForceLockonOnJustAvoid  JustAvoid 시 상대에게 락온을 강제할지 여부
	 */
	static void ProcessHits(AActor* Instigator, const TArray<FHitResult>& RawHits, const FName& DamageRow, TFunctionRef<void(const TArray<FHitResult>&)> OnValidated, bool bForceLockonOnJustAvoid = false);
	
	static void SpawnEffects(UObject* WorldContextObject, const FName& DamageRow, const FVector& Location, const FRotator& Rotation);

	static FRotator GetEventRotation(EYSDirectionPolicy DirectionPolicy, AActor* OwnerActor, const FName& SocketName, AActor* PlaybackTarget = nullptr);
	static FVector GetEventPosition(EYSPositionPolicy PositionPolicy, AActor* OwnerActor, const FName& SocketName, const FVector& RelativeOffset);

	UFUNCTION(BlueprintCallable)
	static FRotator GetAbilityEventRotation(EYSDirectionPolicy DirectionPolicy, UYSGameplayAbility* OwningAbility, const FName& SocketName);

	UFUNCTION(BlueprintCallable)
	static FVector GetAbilityEventPosition(EYSPositionPolicy PositionPolicy, UYSGameplayAbility* OwningAbility,
		const FName& SocketName, const FVector& RelativeOffset);
};

