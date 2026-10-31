// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Combat/YSGroggyRules.h"
#include "Combat/YSSwitchAbusePolicy.h"
#include "Data/YSDataStruct.h"
#include "General/YSStruct.h"
#include "Engine/DeveloperSettings.h"
#include "Input/Combo/YSComboData.h"
#include "Input/StateMachine/YSInputStates.h"
#include "StructUtils/InstancedStruct.h"
#include "YSDeveloperSettings.generated.h"

class UYSCombatProfile;

/**
 *
 */
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "YS Developer Settings"))
class MNYS_API UYSDeveloperSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public :
	UYSDeveloperSettings();

	virtual void PostInitProperties() override;

	static const FYSCommandSequence* GetCommandSequence(const FName& RowName);
	static const FYSDamageInfo* GetDamageInfo(const FName& RowName);

	static TArray<const FYSCommandSequence*> GetAllCommandSequences();

	// 에디터 전용 : 데미지 레지스트리에 등록된 모든 Row 이름을 반환합니다. (FName 프로퍼티의 meta=GetOptions 드롭다운 소스)
	UFUNCTION()
	static TArray<FName> GetDamageRowOptions();

	// 입력 상태 정의 표의 행 이름 (FName 프로퍼티의 meta=GetOptions 드롭다운 소스).
	UFUNCTION()
	static TArray<FName> GetInputStateOptions();

	/** 행 구조가 FYSInputStateDefinition 이 아니면 nullptr. */
	const UDataTable* GetInputStateTable() const;
	const FYSInputStateDefinition* FindInputStateDefinition(FName State) const;
	const FYSInputWindowRule* FindInputWindowRule(FName State) const;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Input", meta = (DisplayName = "입력 상태 정의 표", RequiredAssetDataTags = "RowStructure=/Script/MNYS.YSInputStateDefinition"))
	TSoftObjectPtr<UDataTable> InputStateTable;
	
public : 
	UPROPERTY(EditAnywhere, Config, Category = "JustAvoidGameplayEffect")
	TSubclassOf<class UGameplayEffect> JustAvoidGameplayEffect;
	
	UPROPERTY(EditAnywhere, Config, Category = "YS | StatInitEffect")
	TSubclassOf<class UGameplayEffect> StatInitEffect;
	
	UPROPERTY(EditAnywhere, Config, Category = "YS | StatDeriveEffect")
	TSubclassOf<class UGameplayEffect> StatDeriveEffect;

	float GetLevelHpScale(const UWorld* World) const;

	// 표에 없는 레벨은 1 — 체력을 바꾸지 않는다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Level", meta = (DisplayName = "레벨별 체력 배율"))
	TMap<TSoftObjectPtr<UWorld>, float> LevelHpScales;

	/** 행 구조가 FYSStageDefinition 이 아니면 nullptr. */
	const UDataTable* GetStageTable() const;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Stage", meta = (DisplayName = "스테이지 정의 표", RequiredAssetDataTags = "RowStructure=/Script/MNYS.YSStageDefinition"))
	TSoftObjectPtr<UDataTable> StageTable;

	// 표의 행이 이보다 적으면 있는 행만 쓴다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Stage", meta = (DisplayName = "런당 스테이지 수", ClampMin = "1"))
	int32 RunStageCount = 3;

	// 플레이어와 구역 자리 상자 사이 거리(cm).
	UPROPERTY(EditAnywhere, Config, Category = "YS | Stage", meta = (DisplayName = "구역 띄우는 거리", ClampMin = "0"))
	float ZoneLoadDistance = 3000.f;

	// 띄우는 거리보다 커야 경계에서 띄우고 내리기를 반복하지 않는다(cm).
	UPROPERTY(EditAnywhere, Config, Category = "YS | Stage", meta = (DisplayName = "구역 내리는 거리", ClampMin = "0"))
	float ZoneUnloadDistance = 4500.f;

#pragma region Combat
	bool ShouldOpenSwitchWindow(EYSSwitchTrigger Trigger) const;

	/** 상태별 덮어쓰기 중 ASC 가 처음 맞는 행의 정책, 없으면 기본 정책. nullptr 이면 제어하지 않는다. */
	const FYSSwitchAbusePolicy* ResolveSwitchAbusePolicy(const class UAbilitySystemComponent* ASC) const;

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

	// 행 = 진입기 형 (FYSSwitchEntryForm). 세 형이 진입기 모션 1개를 같이 쓰고 이 표만 다르다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "스위치 어택 진입기 형 표", RequiredAssetDataTags = "RowStructure=/Script/MNYS.YSSwitchEntryForm"))
	TSoftObjectPtr<UDataTable> SwitchEntryFormTable;

	// 등급의 임팩트 행. 없거나 None 이면 nullptr — 임팩트 없음.
	const FYSImpactProfile* GetImpactProfile(EYSImpactGrade Grade) const;

	// 스위치 창이 열린 순간(타겟 바꿈)의 임팩트 등급. 트리거가 표에 없으면 None.
	EYSImpactGrade GetSwitchWindowImpact(EYSSwitchTrigger Trigger) const;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "타겟 높이 · 방향 구간 경계"))
	FYSTargetBandRules TargetBandRules;

	// 피해 행의 임팩트 칸이 여기의 등급을 고른다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "[용어 제안] 임팩트 행 (등급별)"))
	TMap<EYSImpactGrade, FYSImpactProfile> ImpactProfiles;

	// 스위치 창이 열린 순간(Q/E 타겟 전환 · 태그 교대) 걸 임팩트 등급. 표에 없는 트리거는 임팩트 없음.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "[용어 제안] 타겟 바꿈 임팩트 (트리거별)"))
	TMap<EYSSwitchTrigger, EYSImpactGrade> SwitchWindowImpact;

	// 패시브 상대평가를 가진 공격자가 자기보다 HP 비율이 높은 대상을 칠 때 곱한다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "상대평가 피해 배율", ClampMin = "0"))
	float RelativeEvaluationBonus = 1.15f;

	// 입력 창이 닫히는 신호가 오지 않아도 이 시간이 지나면 무입력 형으로 나간다. 스위치 창 GE 길이와 맞춘다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Combat", meta = (DisplayName = "진입기 입력 창 상한 (초)", ClampMin = "0"))
	float SwitchEntryInputWindowLimit = 0.5f;
#pragma endregion

#pragma region Squad
	UPROPERTY(EditAnywhere, Config, Category = "YS | Squad", meta = (DisplayName = "태그 쿨타임 (초)", ClampMin = "0"))
	float TagCooldown = 8.f;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Squad", meta = (DisplayName = "태그 게이지 최대", ClampMin = "0"))
	float MaxTagGauge = 3.f;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Squad", meta = (DisplayName = "스위치 어택 게이지 소모", ClampMin = "0"))
	float SwitchAttackCost = 1.f;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Squad", meta = (DisplayName = "교대 카메라 블렌드 (초)", ClampMin = "0"))
	float TagSwapCameraBlendTime = 0.15f;
#pragma endregion

#pragma region Echo
	UPROPERTY(EditAnywhere, Config, Category = "YS | Echo", meta = (DisplayName = "분신 캡슐 콜리전 프로필"))
	FName EchoCollisionProfile = TEXT("EchoBody");
#pragma endregion

#pragma region Navigation
	// 위치를 NavMesh 에 붙일 때 위아래로 찾는 거리.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Navigation", meta = (DisplayName = "내비 투영 높이 (cm)", ClampMin = "0"))
	float NavProjectionMaxHeight = 10000.f;
#pragma endregion

#pragma region Animation
	// 이동 방향 각(절댓값)이 이 이하면 정면, 후면 하한 이상이면 후면, 사이는 좌우.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Animation", meta = (DisplayName = "정면 판정 상한 (도)", ClampMin = "0", ClampMax = "180"))
	float FrontwardMaxAngle = 65.f;

	UPROPERTY(EditAnywhere, Config, Category = "YS | Animation", meta = (DisplayName = "후면 판정 하한 (도)", ClampMin = "0", ClampMax = "180"))
	float BackwardMinAngle = 115.f;
#pragma endregion

#pragma region Input
	UPROPERTY(EditAnywhere, Config, Category = "YS | Input", meta = (DisplayName = "입력 저장 슬롯 이름"))
	FString InputSaveSlotName = TEXT("InputSaveSlot");
#pragma endregion

#pragma region Sequence
	UPROPERTY(EditAnywhere, Config, Category = "YS | Sequence", meta = (DisplayName = "시전자 바인딩 태그"))
	FName SequenceInstigatorBindingTag = TEXT("Player");

	// 시퀀스에 이 태그의 바인딩이 있을 때만 대상을 붙인다.
	UPROPERTY(EditAnywhere, Config, Category = "YS | Sequence", meta = (DisplayName = "대상 바인딩 태그"))
	FName SequenceTargetBindingTag = TEXT("Enemy");
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
