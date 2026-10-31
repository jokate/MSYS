// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSEnum.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Templates/SubclassOf.h"
#include "YSBlueprintFunctionLibrary.generated.h"

struct FYSDamageEffectContext;
struct FYSDamageInfo;
struct FYSAbilityHitContext;
struct FYSSpawnActorConfig;
struct FYSTransformPolicyContext;
class UYSGameplayAbility;
class UAbilitySystemComponent;
class UYSCharacterAttributeSetBase;
class UWorld;

UCLASS()
class MNYS_API UYSBlueprintFunctionLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public :
	/** @param EffectScale  고정 피해를 뺀 부분에만 곱한다. */
	UFUNCTION(BlueprintPure)
	static float GetFinalDamage(const UYSCharacterAttributeSetBase* Owner, const UYSCharacterAttributeSetBase* Target, const FName& SkillID, float EffectScale = 1.f);

	/** 피해 입구 검사. 아군 → 사망 → ASC · 속성 유효 → 저스트 회피 창 → 무적 순으로 거른다. 저스트 회피 창이면 피해 대신 저스트 회피 GE 를 부여하고 false. JustAvoidGranted 는 연속 히트처럼 같은 대상을 반복 검사하는 호출자의 기록이다 (창 안에서는 GE 를 한 번만 걸고 창 밖이면 지운다. nullptr 이면 매번 건다). true 면 피해 · 히트 이벤트 · 이펙트를 보내도 된다. */
	static bool AdmitHit(AActor* Instigator, AActor* Target, TSet<TWeakObjectPtr<AActor>>* JustAvoidGranted = nullptr);

	/** @param EffectScale  HP 피해(고정 피해 제외)와 그로기 피해에 곱한다 (스위치 남용 감쇠 등). 무적 대상에게는 아무것도 보내지 않는다. */
	UFUNCTION(BlueprintCallable)
	static void SendHitEventToTarget(AActor* Instigator, AActor* Target, const FName& SkillID, float EffectScale = 1.f);

	/** 이 대상에게 실제로 쓸 피해 행. 등급 0 · 1 몬스터이고 행에 대체 행이 있으면 그 행, 아니면 그대로. */
	static FName ResolveDamageRow(const AActor* Target, const FName& DamageRow);
	
	static void SpawnEffects(UObject* WorldContextObject, const FName& DamageRow, const FVector& Location, const FRotator& Rotation);

	/** 공격 1회의 임팩트(시간 · 셰이크 · 줌)를 건다. 맞은 대상 수와 상관없이 한 번이라 대상 루프 밖에서 부른다. 등급은 피해 행의 임팩트 칸이고 None 이면 아무것도 하지 않는다. */
	static void PlayHitImpact(AActor* Instigator, const TArray<AActor*>& Targets, const FName& DamageRow);

	/** 등급을 직접 줄 때(스위치 창 열림). 행 조회와 채널 재생은 PlayHitImpact 와 같다. */
	static void PlayImpact(AActor* Instigator, const TArray<AActor*>& Targets, EYSImpactGrade Grade);
	
	/** 풀에서 꺼내거나 새로 스폰한다. 어느 쪽이든 FinishSpawning 까지 끝난 액터가 나온다. */
	static AActor* AcquirePooledActor(UWorld* World, TSubclassOf<AActor> ActorClass, const FTransform& SpawnTransform);
	
	/** 공격 액터를 내는 입구는 이것 하나다. 스폰 지연이 있으면 nullptr 를 돌려주고 지연 뒤에 낸다. */
	static AActor* SpawnByConfig(UObject* WorldContext, const FYSSpawnActorConfig& Config, const FYSTransformPolicyContext& PolicyContext, AActor* AttachParent = nullptr, const TSharedPtr<FYSAbilityHitContext>& HitContext = nullptr);
	static FTransform CalculateSpawnTransform(UObject* WorldContext, const FYSSpawnActorConfig& Config, const FYSTransformPolicyContext& PolicyContext);
	static void ApplyHitEffects(AActor* Source, AActor* Instigator, AActor* Target, const FName& SkillID, const FHitResult& HitResult);

	/** 분신 · 판정 액터면 피해의 주인(Master), 아니면 그대로. */
	static AActor* ResolveDamageInstigator(AActor* Actor);

	/** 임팩트는 공격 1회에 한 번이라 여기 넣지 않고 호출자가 대상 루프 밖에서 부른다. */
	static bool ApplyHitToTarget(AActor* Source, AActor* DamageInstigator, AActor* Target, const FName& DamageRow, float EffectScale,
		const FHitResult& HitResult, const FRotator& EffectRotation, TSet<TWeakObjectPtr<AActor>>* JustAvoidGranted = nullptr);
};

