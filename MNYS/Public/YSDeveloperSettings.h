// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/YSGroggyRules.h"
#include "Combat/YSContextSwitchVariant.h"
#include "Combat/YSSwitchAbusePolicy.h"
#include "Data/YSDataStruct.h"
#include "General/YSStruct.h"
#include "Engine/DeveloperSettings.h"
#include "Input/Combo/YSComboData.h"
#include "StructUtils/InstancedStruct.h"
#include "YSDeveloperSettings.generated.h"

class UYSCombatProfile;

/**
 * 창(저스트 회피 · 스위치)처럼 GE 로 잠깐 얹히는 입력 상태의 입력 규칙.
 * 창마다 다른 것은 이 값뿐이라 상태 클래스를 늘리지 않고 설정 한 줄로 둔다.
 */
USTRUCT()
struct FYSInputWindowRule
{
	GENERATED_BODY()

	// 누르면 창을 닫는 입력 (상태 접두 없는 원본). 어빌리티 라우팅이 끝난 뒤 닫는다.
	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "창을 닫는 입력", Categories = "Input"))
	TArray<FGameplayTag> CloseInputs;

	// 조준(Input.Aim)을 누르고 있는 동안 창이 아니라 조준 상태로 해석할 입력.
	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "조준 중엔 조준 상태로 넘길 입력", Categories = "Input"))
	TArray<FGameplayTag> AimHeldPassThroughInputs;

	// 창 상태에서 아무 어빌리티도 받지 않았을 때 창 아래 상태로 다시 해석할 입력. 창은 그 뒤에 닫는다.
	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "못 받으면 아래 상태로 넘길 입력", Categories = "Input"))
	TArray<FGameplayTag> UnhandledFallThroughInputs;

	// 창이 겹치면 값이 큰 창이 위에 남는다. 낮은 창은 그 아래에 있다가 높은 창이 닫히면 드러난다. 같으면 나중에 열린 창이 위.
	UPROPERTY(EditAnywhere, Category = "YS | Input", meta = (DisplayName = "겹칠 때 우선순위"))
	int32 Priority = 0;
};

/**
 *
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "YS Developer Settings"))
class MNYS_API UYSDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public :
	UYSDeveloperSettings();

	static const FYSCommandSequence* GetCommandSequence(const FName& RowName);
	static const FYSDamageInfo* GetDamageInfo(const FName& RowName);

	static TArray<const FYSCommandSequence*> GetAllCommandSequences();

	// 에디터 전용 : 데미지 레지스트리에 등록된 모든 Row 이름을 반환합니다. (FName 프로퍼티의 meta=GetOptions 드롭다운 소스)
	UFUNCTION()
	static TArray<FName> GetDamageRowOptions();
	
public : 
	UPROPERTY(EditAnywhere, Config, Category = "JustAvoidGameplayEffect")
	TSubclassOf<class UGameplayEffect> JustAvoidGameplayEffect;
	
	UPROPERTY(EditAnywhere, Config, Category = "YS | StatInitEffect")
	TSubclassOf<class UGameplayEffect> StatInitEffect;
	
	UPROPERTY(EditAnywhere, Config, Category = "YS | StatDeriveEffect")
	TSubclassOf<class UGameplayEffect> StatDeriveEffect;

#pragma region Combat
	bool ShouldOpenSwitchWindow(EYSSwitchTrigger Trigger) const;

	/** 상태별 덮어쓰기 중 ASC 가 처음 맞는 행의 정책, 없으면 기본 정책. nullptr 이면 제어하지 않는다. */
	const FYSSwitchAbusePolicy* ResolveSwitchAbusePolicy(const class UAbilitySystemComponent* ASC) const;
	const FYSInputWindowRule* FindInputWindowRule(EYSInputStatesType State) const { return InputWindowRules.Find(State); }

	const struct FYSSwitchEntryForm* FindSwitchEntryForm(FName RowName) const;

	/** 이 입력을 받는 형의 행 이름. InputTag 가 비었으면 무입력 형. 없으면 NAME_None. */
	FName FindSwitchEntryFormName(const FGameplayTag& InputTag) const;

	// 캐릭터 정의에 전투 프로필이 없을 때 쓴다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "기본 전투 프로필"))
	TSoftObjectPtr<UYSCombatProfile> DefaultCombatProfile;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "스위치 창을 여는 트리거"))
	EYSSwitchWindowTriggerRule SwitchWindowTriggerRule = EYSSwitchWindowTriggerRule::TargetSwitchOnly;

	// 입력 상태 Switch 로 바꾸는 GE (UYSInputStateGEComponent). 지속 시간이 곧 창 길이다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "스위치 창 GE"))
	TSubclassOf<class UGameplayEffect> SwitchWindowEffect;

	/** 스위치 창 GE 의 고정 지속(창 상한). 고정값으로 읽을 수 없으면 0. */
	float GetSwitchWindowDuration() const;

	/** 이 트리거로 연 창의 조기 닫기 조건. 없으면 창 GE 지속(상한)으로만 닫힌다. */
	const struct FYSSwitchWindowCloseCondition* FindSwitchWindowCloseCondition(EYSSwitchTrigger Trigger) const;

	// 트리거마다 창을 상한보다 먼저 닫을 조건. 항목이 없는 트리거는 상한까지 열린다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "스위치 창 조기 닫기 조건", BaseStruct = "/Script/MNYS.YSSwitchWindowCloseCondition", ExcludeBaseStruct))
	TMap<EYSSwitchTrigger, FInstancedStruct> SwitchWindowCloseConditions;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "스위치 남용 제어", BaseStruct = "/Script/MNYS.YSSwitchAbusePolicy", ExcludeBaseStruct))
	FInstancedStruct SwitchAbusePolicy;

	// 위에서부터 처음 맞는 행이 기본 정책을 대신한다. 비어 있으면 어떤 상태에서도 기본 정책이다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "스위치 남용 제어 — 상태별 덮어쓰기"))
	TArray<FYSSwitchAbusePolicyOverride> SwitchAbusePolicyOverrides;

	// 트리거별이면 Q/E 로 연 창의 쿨이 태그 스위치 어택을 막지 않는다 (반대도 같다).
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "스위치 남용 기록 범위"))
	EYSSwitchAbuseScope SwitchAbuseScope = EYSSwitchAbuseScope::PerTrigger;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "창 상태 입력 규칙"))
	TMap<EYSInputStatesType, FYSInputWindowRule> InputWindowRules;

	// 행 = 진입기 형 (FYSSwitchEntryForm). 세 형이 진입기 모션 1개를 같이 쓰고 이 표만 다르다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "스위치 어택 진입기 형 표", RequiredAssetDataTags = "RowStructure=/Script/MNYS.YSSwitchEntryForm"))
	TSoftObjectPtr<UDataTable> SwitchEntryFormTable;

	const struct FYSContextSwitchVariant* FindContextSwitchVariant(FName RowName) const;

	// 등급의 임팩트 행. 없거나 None 이면 nullptr — 임팩트 없음.
	const FYSImpactProfile* GetImpactProfile(EYSImpactGrade Grade) const;

	// 스위치 창이 열린 순간(타겟 바꿈)의 임팩트 등급. 트리거가 표에 없으면 None.
	EYSImpactGrade GetSwitchWindowImpact(EYSSwitchTrigger Trigger) const;

	// 행 = Q/E 수동 전환 창 안 입력의 변형 (FYSContextSwitchVariant). 진입기 형 표와 따로 둔다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "컨텍스트 스위치 변형 표", RequiredAssetDataTags = "RowStructure=/Script/MNYS.YSContextSwitchVariant"))
	TSoftObjectPtr<UDataTable> ContextSwitchVariantTable;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "컨텍스트 스위치 상황 판정 경계"))
	FYSContextSwitchJudgeRules ContextSwitchJudgeRules;

	// 임팩트 행 = 등급. 시간(히트스톱 · 슬로우) · 셰이크 · 줌의 강도. COMBAT_TONE_GUIDE §3. 피해 행의 임팩트 칸이 등급을 고른다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "[용어 제안] 임팩트 행 (등급별)"))
	TMap<EYSImpactGrade, FYSImpactProfile> ImpactProfiles;

	// 스위치 창이 열린 순간(Q/E 타겟 전환 · 태그 교대) 걸 임팩트 등급. 표에 없는 트리거는 임팩트 없음.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "[용어 제안] 타겟 바꿈 임팩트 (트리거별)"))
	TMap<EYSSwitchTrigger, EYSImpactGrade> SwitchWindowImpact;
#pragma endregion

#pragma region Squad
	UPROPERTY(EditAnywhere, Config, Category = "YS | Squad", meta = (DisplayName = "태그 쿨타임 (초)", ClampMin = "0"))
	float TagCooldown = 8.f;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Squad", meta = (DisplayName = "태그 게이지 최대", ClampMin = "0"))
	float MaxTagGauge = 3.f;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Squad", meta = (DisplayName = "스위치 어택 게이지 소모", ClampMin = "0"))
	float SwitchAttackCost = 1.f;
#pragma endregion

#pragma region Groggy
	// 몬스터에 그로기 프로필이 없을 때 쓴다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Groggy", meta = (DisplayName = "기본 그로기 규칙"))
	FYSGroggyRules DefaultGroggyRules;

	// 기본은 UYSGE_GroggyBreak. Cue 등을 더하려면 그 BP 파생으로 바꾼다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Groggy", meta = (DisplayName = "브레이크 GE"))
	TSubclassOf<class UGameplayEffect> GroggyBreakEffect;
#pragma endregion

private : 
	template <class T>
	static const T* GetRegistryData(const FName& RegistryName, const FName& RowName);
	
	template <class T>
	static TArray<const T*> GetAllRegistryData(const FName& RegistryName);
	
protected :
	UPROPERTY(EditAnywhere, Config, Category = "Data Registry")
	FName CommandRegistryName;

	UPROPERTY(EditAnywhere, Config, Category = "Data Registry")
	FName DamageRegistryName;
	
};
