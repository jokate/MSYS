// Fill out your copyright notice in the Description page of Project Settings.


#include "YSDeveloperSettings.h"

#include "AbilitySystemComponent.h"
#include "DataRegistry.h"
#include "DataRegistryId.h"
#include "DataRegistrySubsystem.h"
#include "GameplayEffect.h"
#include "Ability/GameplayEffect/YSGE_GroggyBreak.h"
#include "Combat/YSSwitchAbusePolicy.h"
#include "Combat/YSSwitchEntryForm.h"
#include "Combat/YSSwitchWindowCloseCondition.h"
#include "Data/YSCombatProfile.h"
#include "General/YSInputGameplayTags.h"

namespace
{
	// ini 의 맵 한 줄은 생성자 값을 통째로 덮는다. 맵에 없는 등급은 조회 때 이 표로 돌아온다. 새 등급은 여기 한 줄.
	const TMap<EYSImpactGrade, FYSImpactProfile>& GetCodeDefaultImpactProfiles()
	{
		static const TMap<EYSImpactGrade, FYSImpactProfile> Defaults = []()
		{
			// 히트스톱만 있던 때의 값이다. 셰이크 · 줌은 비워 둔다 — 값은 사장님이 정한다.
			const TPair<EYSImpactGrade, float> HitStops[] = {
				{ EYSImpactGrade::S, 0.05f },
				{ EYSImpactGrade::M, 0.08f },
				{ EYSImpactGrade::MPlus, 0.10f },
				{ EYSImpactGrade::L, 0.12f },
				{ EYSImpactGrade::LPlus, 0.25f },
			};

			TMap<EYSImpactGrade, FYSImpactProfile> Result;

			for ( const TPair<EYSImpactGrade, float>& HitStop : HitStops )
			{
				FYSImpactProfile Profile;
				Profile.TimeDuration = HitStop.Value;
				Result.Add(HitStop.Key, Profile);
			}

			return Result;
		}();
		return Defaults;
	}

	// 에디터 값과 실제 값이 다를 때 찾을 수 있게 키마다 처음 한 번만 알린다.
	bool ShouldReportFallback(EYSImpactGrade Grade)
	{
		static TSet<EYSImpactGrade> Reported;
		bool bAlreadyReported = false;
		Reported.Add(Grade, &bAlreadyReported);
		return bAlreadyReported == false;
	}
}

UYSDeveloperSettings::UYSDeveloperSettings()
{
	// 기획 미결 항목의 기본값 (S4 제어 = ① 전역 쿨, S3 = A 파생만 창 소비 · ADS 중 LMB = 사격).
	// 태그는 컨테이너가 아니라 배열로 둔다 — CDO 생성 시점에 태그 매니저를 건드리지 않기 위해서다.
	SwitchAbusePolicy.InitializeAs<FYSSwitchAbusePolicy_GlobalCooldown>();

	// 수동 Q/E 창만 전환 회전이 끝나면 닫는다 (재정의 §9). 태그 교대 창은 상한까지.
	SwitchWindowCloseConditions.Add(EYSSwitchTrigger::TargetSwitch, FInstancedStruct::Make<FYSSwitchWindowCloseCondition_ControlYawAligned>());

	FYSInputWindowRule JustAvoidRule;
	JustAvoidRule.CloseInputs = { YSInputTags::InputAttack, YSInputTags::InputSkill1, YSInputTags::InputSkill2 };
	JustAvoidRule.AimHeldPassThroughInputs = { YSInputTags::InputAttack };
	JustAvoidRule.Priority = 10;	// 저스트 창이 스위치 창을 덮는다 (위험 지도 E8).
	InputWindowRules.Add(EYSInputStatesType::JustAvoid, JustAvoidRule);

	FYSInputWindowRule SwitchRule;
	SwitchRule.CloseInputs = { YSInputTags::InputAttack, YSInputTags::InputSkill1, YSInputTags::InputSkill2 };
	// 창 안 공격은 늘 무언가 나간다 — 변형이 안 맞으면 평소 입력으로.
	SwitchRule.UnhandledFallThroughInputs = { YSInputTags::InputAttack, YSInputTags::InputSkill1, YSInputTags::InputSkill2 };
	InputWindowRules.Add(EYSInputStatesType::Switch, SwitchRule);

	GroggyBreakEffect = UYSGE_GroggyBreak::StaticClass();

	ImpactProfiles = GetCodeDefaultImpactProfiles();
}

bool UYSDeveloperSettings::ShouldOpenSwitchWindow(EYSSwitchTrigger Trigger) const
{
	switch ( SwitchWindowTriggerRule )
	{
	case EYSSwitchWindowTriggerRule::Both :
		return true;
	case EYSSwitchWindowTriggerRule::TargetSwitchOnly :
		return Trigger == EYSSwitchTrigger::TargetSwitch;
	case EYSSwitchWindowTriggerRule::TagSwapOnly :
		return Trigger == EYSSwitchTrigger::TagSwap;
	default :
		return false;
	}
}

float UYSDeveloperSettings::GetSwitchWindowDuration() const
{
	const UGameplayEffect* WindowEffect = SwitchWindowEffect ? SwitchWindowEffect->GetDefaultObject<UGameplayEffect>() : nullptr;
	float Duration = 0.f;

	if ( WindowEffect == nullptr || WindowEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(1.f, Duration) == false )
	{
		return 0.f;
	}

	return Duration;
}

const FYSSwitchAbusePolicy* UYSDeveloperSettings::ResolveSwitchAbusePolicy(const UAbilitySystemComponent* ASC) const
{
	if ( IsValid(ASC) )
	{
		for ( const FYSSwitchAbusePolicyOverride& Override : SwitchAbusePolicyOverrides )
		{
			if ( ASC->HasAnyMatchingGameplayTags(Override.RequiredTags) )
			{
				return Override.Policy.GetPtr<FYSSwitchAbusePolicy>();
			}
		}
	}

	return SwitchAbusePolicy.GetPtr<FYSSwitchAbusePolicy>();
}

const FYSSwitchWindowCloseCondition* UYSDeveloperSettings::FindSwitchWindowCloseCondition(EYSSwitchTrigger Trigger) const
{
	const FInstancedStruct* Found = SwitchWindowCloseConditions.Find(Trigger);
	return Found ? Found->GetPtr<FYSSwitchWindowCloseCondition>() : nullptr;
}

const FYSSwitchEntryForm* UYSDeveloperSettings::FindSwitchEntryForm(FName RowName) const
{
	const UDataTable* Table = SwitchEntryFormTable.LoadSynchronous();

	if ( IsValid(Table) == false || RowName.IsNone() )
	{
		return nullptr;
	}

	return Table->FindRow<FYSSwitchEntryForm>(RowName, TEXT("SwitchEntryForm"), false);
}

const FYSContextSwitchVariant* UYSDeveloperSettings::FindContextSwitchVariant(FName RowName) const
{
	const UDataTable* Table = ContextSwitchVariantTable.LoadSynchronous();

	if ( IsValid(Table) == false || RowName.IsNone() )
	{
		return nullptr;
	}

	return Table->FindRow<FYSContextSwitchVariant>(RowName, TEXT("ContextSwitchVariant"), false);
}

const FYSImpactProfile* UYSDeveloperSettings::GetImpactProfile(EYSImpactGrade Grade) const
{
	if ( Grade == EYSImpactGrade::None )
	{
		return nullptr;
	}

	if ( const FYSImpactProfile* Found = ImpactProfiles.Find(Grade) )
	{
		return Found;
	}

	const FYSImpactProfile* Default = GetCodeDefaultImpactProfiles().Find(Grade);

	if ( ShouldReportFallback(Grade) )
	{
		if ( Default )
		{
			UE_LOG(LogTemp, Warning, TEXT("[Impact] %s 등급이 YS Developer Settings 임팩트 행에 없어 코드 기본값(시간 %.3f 초)을 쓴다."), *UEnum::GetValueAsString(Grade), Default->TimeDuration);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("[Impact] %s 등급이 설정에도 코드 기본값 표에도 없어 임팩트가 없다."), *UEnum::GetValueAsString(Grade));
		}
	}

	return Default;
}

EYSImpactGrade UYSDeveloperSettings::GetSwitchWindowImpact(EYSSwitchTrigger Trigger) const
{
	const EYSImpactGrade* Found = SwitchWindowImpact.Find(Trigger);
	return Found ? *Found : EYSImpactGrade::None;
}

FName UYSDeveloperSettings::FindSwitchEntryFormName(const FGameplayTag& InputTag) const
{
	const UDataTable* Table = SwitchEntryFormTable.LoadSynchronous();

	if ( IsValid(Table) == false || Table->GetRowStruct() == nullptr || Table->GetRowStruct()->IsChildOf(FYSSwitchEntryForm::StaticStruct()) == false )
	{
		UE_LOG(LogTemp, Warning, TEXT("[SwitchEntry] 진입기 형 표가 비었거나 행 구조가 다르다. YS Developer Settings 에서 채워야 한다."));
		return NAME_None;
	}

	for ( const TPair<FName, uint8*>& Pair : Table->GetRowMap() )
	{
		const FYSSwitchEntryForm* Form = reinterpret_cast<const FYSSwitchEntryForm*>(Pair.Value);

		if ( InputTag.IsValid() ? Form->AcceptsInput(InputTag) : Form->IsNoInputForm() )
		{
			return Pair.Key;
		}
	}

	return NAME_None;
}

template <class T>
const T* UYSDeveloperSettings::GetRegistryData(const FName& RegistryName, const FName& RowName)
{
	FDataRegistryId RegistryId;
	RegistryId.RegistryType = RegistryName;
	RegistryId.ItemName = RowName;

	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	
	if ( IsValid(DataRegistrySubsystem) == false )
	{
		return nullptr;
	}

	return DataRegistrySubsystem->GetCachedItem<T>(RegistryId);
}

template <class T>
TArray<const T*> UYSDeveloperSettings::GetAllRegistryData(const FName& RegistryName)
{
	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	
	if ( IsValid(DataRegistrySubsystem) == false )
	{
		return TArray<const T*>();
	}
	
	UDataRegistry* Registry = DataRegistrySubsystem->GetRegistryForType(RegistryName);
	
	if ( IsValid(Registry) == false )
		return TArray<const T*>();
	
	TArray<const T*> AllRegistry;
	Registry->GetAllItems<T>(TEXT("Preload"), AllRegistry);

	return AllRegistry;
}

const FYSCommandSequence* UYSDeveloperSettings::GetCommandSequence(const FName& RowName)
{
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	if (IsValid(Settings) == false || Settings->CommandRegistryName.IsNone())
	{
		return nullptr;
	}

	return GetRegistryData<FYSCommandSequence>(Settings->CommandRegistryName, RowName);
}

const FYSDamageInfo* UYSDeveloperSettings::GetDamageInfo(const FName& RowName)
{
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	if (IsValid(Settings) == false || Settings->DamageRegistryName.IsNone())
	{
		return nullptr;
	}

	return GetRegistryData<FYSDamageInfo>(Settings->DamageRegistryName, RowName);
}

TArray<const FYSCommandSequence*> UYSDeveloperSettings::GetAllCommandSequences()
{
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	if (IsValid(Settings) == false || Settings->CommandRegistryName.IsNone())
	{
		return TArray<const FYSCommandSequence*>();
	}
	
	return GetAllRegistryData<FYSCommandSequence>(Settings->CommandRegistryName);
}

TArray<FName> UYSDeveloperSettings::GetDamageRowOptions()
{
	TArray<FName> RowNames;

	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();
	if (IsValid(Settings) == false || Settings->DamageRegistryName.IsNone())
	{
		return RowNames;
	}

	UDataRegistrySubsystem* DataRegistrySubsystem = UDataRegistrySubsystem::Get();
	if (IsValid(DataRegistrySubsystem) == false)
	{
		return RowNames;
	}

	UDataRegistry* Registry = DataRegistrySubsystem->GetRegistryForType(Settings->DamageRegistryName);
	if (IsValid(Registry) == false)
	{
		return RowNames;
	}

	// 아이템 캐싱(Acquire) 없이 소스(DataTable 등)에서 가능한 ID를 그대로 열거 → 에디터에서도 안전.
	TArray<FDataRegistryId> RegistryIds;
	Registry->GetPossibleRegistryIds(RegistryIds);

	RowNames.Reserve(RegistryIds.Num());
	for (const FDataRegistryId& RegistryId : RegistryIds)
	{
		RowNames.Add(RegistryId.ItemName);
	}

	return RowNames;
}
