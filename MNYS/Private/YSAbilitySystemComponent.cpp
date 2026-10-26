// Fill out your copyright notice in the Description page of Project Settings.


#include "YSAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "YSDeveloperSettings.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/YSGameplayEffectHandler.h"
#include "Character/YSCharacterPlayer.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "Character/Components/YSCharacterMovementComponent.h"
#include "Data/YSAbilityDataAsset.h"
#include "General/YSGameplayTag.h"
#include "General/YSGeneratedGameplayTags.h"
#include "General/YSStruct.h"
#include "Input/YSInputSaveData.h"
#include "Kismet/GameplayStatics.h"
#include "Subsystem/YSSquadSubsystem.h"


// Sets default values for this component's properties
UYSAbilitySystemComponent::UYSAbilitySystemComponent()
{

}

UYSAbilitySystemComponent* UYSAbilitySystemComponent::Get(AActor* Owner)
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Owner);

	if ( IsValid(ASC) == false )
		return nullptr;

	return Cast<UYSAbilitySystemComponent>(ASC);
}

void UYSAbilitySystemComponent::BeginPlay()
{
	Super::BeginPlay();
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &ThisClass::OnEffectAppliedToSelf);
}

void UYSAbilitySystemComponent::OnEffectAppliedToSelf(UAbilitySystemComponent* Source, const FGameplayEffectSpec& Spec, FActiveGameplayEffectHandle Handle)
{
	// 저스트 회피 판정 GE 는 YSTraceObject 가 공격자를 Instigator 로 넣어 건다. 그 순간이 공격자를 알 수 있는 유일한 지점이다.
	if ( Spec.GetDynamicAssetTags().HasTagExact(YSTags::Event_JustAvoid) == false )
		return;

	LastJustAvoidInstigator = Spec.GetContext().GetInstigator();
}

void UYSAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SaveInputBindings();
	Super::EndPlay(EndPlayReason);
}

void UYSAbilitySystemComponent::SaveInputBindings()
{
	AYSCharacterPlayer* Player = Cast<AYSCharacterPlayer>(GetOwner());
	if (IsValid(Player) == false)
	{
		return;
	}

	// 스쿼드 폰은 각자 EndPlay 를 맞는다. 필드 폰 한 번만 세 슬롯을 모아 저장한다.
	// 스펙은 EndPlay 가 아니라 OnUnregister 에서 비워지므로 먼저 끝난 벤치 폰 스펙도 아직 읽힌다.
	TArray<UYSAbilitySystemComponent*> Sources;
	const UYSSquadSubsystem* Squad = Player->GetGameInstance() ? Player->GetGameInstance()->GetSubsystem<UYSSquadSubsystem>() : nullptr;

	if (IsValid(Squad) && Squad->GetSlotCount() > 0)
	{
		if (Squad->GetActivePawn() != Player)
		{
			return;
		}

		for (int32 Index = 0; Index < Squad->GetSlotCount(); ++Index)
		{
			FYSCharacterInfo Definition;
			UYSAbilitySystemComponent* SlotASC = nullptr;

			if (Squad->GetSlotInfo(Index, Definition, SlotASC) && IsValid(SlotASC))
			{
				Sources.Add(SlotASC);
			}
		}
	}
	else
	{
		Sources.Add(this);
	}

	// 이번 스쿼드에 없는 캐릭터의 바인딩을 지우지 않도록 기존 저장에 덮어 쓴다.
	UYSInputSaveData* SaveGameInstance = Cast<UYSInputSaveData>(UGameplayStatics::LoadGameFromSlot("InputSaveSlot", 0));
	if (IsValid(SaveGameInstance) == false)
	{
		SaveGameInstance = Cast<UYSInputSaveData>(UGameplayStatics::CreateSaveGameObject(UYSInputSaveData::StaticClass()));
	}

	if (IsValid(SaveGameInstance) == false)
	{
		return;
	}

	for (UYSAbilitySystemComponent* Source : Sources)
	{
		for ( FGameplayAbilitySpecHandle& AbilitySpecHandle : Source->AbilitySpecHandles )
		{
			FGameplayAbilitySpec* AbilitySpec = Source->FindAbilitySpecFromHandle(AbilitySpecHandle);

			if (AbilitySpec == nullptr || AbilitySpec->Ability == nullptr || AbilitySpec->GetDynamicSpecSourceTags().IsEmpty())
				continue;

			FYSInputSaveDataMemeber InputSaveData(AbilitySpec->GetDynamicSpecSourceTags(), AbilitySpec->Ability->GetClass());
			SaveGameInstance->AddInputSaveData(InputSaveData);
		}
	}

	UGameplayStatics::SaveGameToSlot(SaveGameInstance, "InputSaveSlot", 0);
}

void UYSAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	// 어빌리티에 대한 등록이 이루어진 경우. 더해주고 빼준다. 차후 해당 데이터를 기반으로 해서 어빌리티를 추적하자.
	AbilitySpecHandles.Emplace(AbilitySpec.Handle);
}

void UYSAbilitySystemComponent::GiveInitAbility()
{
	if ( IsValid(GrantAbilityData) == false || bInitAbilityGiven )
		return;

	bInitAbilityGiven = true;

	
	// 테스트 코드 나중에 바꿉시다.
	UYSInputSaveData* LoadedInput = Cast<UYSInputSaveData>(UGameplayStatics::LoadGameFromSlot("InputSaveSlot", 0));
	
	bool bIsValidSaveData = IsValid(LoadedInput);
	
	for ( const FYSGrantedAbilityData& GrantedAbilityData : GrantAbilityData->GetAllAbilities())
	{
		FGameplayAbilitySpec AbilitySpec(GrantedAbilityData.AbilityClass, GrantedAbilityData.Level);

		bool bIsProcessed = false;
		if (bIsValidSaveData)
		{
			FGameplayTagContainer InputTag = LoadedInput->GetInputTagByAbilityClass(GrantedAbilityData.AbilityClass);
			
			if ( InputTag.IsEmpty() == false )
			{
				bIsProcessed = true;
				AbilitySpec.GetDynamicSpecSourceTags().AppendTags(InputTag);
			}
		}
	
		if ( bIsProcessed == false && GrantedAbilityData.InputTag.IsEmpty() == false )
		{
			AbilitySpec.GetDynamicSpecSourceTags().AppendTags(GrantedAbilityData.InputTag);
		}	

		GiveAbility(AbilitySpec);
	}
}

void UYSAbilitySystemComponent::OnTagUpdated(const FGameplayTag& Tag, bool TagExists)
{
	Super::OnTagUpdated(Tag, TagExists);

	if ( OnGameplayTagStateChanged.IsBound() )
	{
		OnGameplayTagStateChanged.Broadcast(Tag, TagExists);
	}
}

void UYSAbilitySystemComponent::AllocateSkillToAbilityTag(const FGameplayTag& SkillTag,
	const TSubclassOf<UGameplayAbility> AbilityClass)
{
	for ( FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitySpecHandles )
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilitySpecHandle);

		if ( AbilitySpec == nullptr )
			continue;

		if ( AbilitySpec->GetDynamicSpecSourceTags().HasTagExact(SkillTag) )
		{
			AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(SkillTag);
		}
			
		if ( AbilitySpec->Ability->GetClass() == AbilityClass )
		{
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(SkillTag);
		}
	}
}

void UYSAbilitySystemComponent::ClearAirUsage()
{
	AirUsedHandles.Reset();
}


void UYSAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	AbilitySpecHandles.Remove(AbilitySpec.Handle);
	Super::OnRemoveAbility(AbilitySpec);
}

int32 UYSAbilitySystemComponent::HandleGameplayEvent(FGameplayTag EventTag, const FGameplayEventData* Payload)
{
	int32 SuperRes =  Super::HandleGameplayEvent(EventTag, Payload);
	
	if ( GrantAbilityData == nullptr )
	{
		return SuperRes;
	}
	
	const FYSGameplayEffectHandler* PassiveEffect =GrantAbilityData->GetPassiveEffect(EventTag);
	
	if ( PassiveEffect != nullptr )
	{
		const FGameplayEffectContextHandle Context = MakeEffectContext();	
		const FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(PassiveEffect->EffectClass, 1.f, Context);
		if (Spec.IsValid())
		{
			for (const TPair<FGameplayTag, float>& Pair : PassiveEffect->Magnitudes)
			{
				Spec.Data->SetSetByCallerMagnitude(Pair.Key, Pair.Value);
			}
			ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
		}
		
		++SuperRes;
	}
	
	return SuperRes;
}

bool UYSAbilitySystemComponent::ProcessSkillActive(const FGameplayTag& InputTag)
{
	const UYSCharacterMovementComponent* Movement = UYSCharacterMovementComponent::Get(GetOwner());
	const bool bAirborne = IsValid(Movement) && Movement->IsFalling();

	for ( FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitySpecHandles )
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilitySpecHandle);

		if ( AbilitySpec == nullptr || AbilitySpec->IsActive() )
			continue;

		if ( AbilitySpec->GetDynamicSpecSourceTags().HasTagExact(InputTag) == false )
			continue;
		
		// 에어본 + 에어본 스킬 사용이 이미 된 경우에는 굳이 사용하지 말자
		if ( bAirborne && AirUsedHandles.Contains(AbilitySpecHandle) )
			continue;

		if ( !TryActivateAbility(AbilitySpecHandle) )
		{
			continue;
		}

		return true;
	}

	return false;
}

bool UYSAbilitySystemComponent::ProcessAlreadyActiveAbility(const FGameplayTag& InputTag, EYSInputPhase InputPhase)
{
	for ( FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitySpecHandles )
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilitySpecHandle);

		if ( AbilitySpec == nullptr )
			continue;

		// 미 작동 중인 어빌리티의 경우에는 굳이 처리하지 말자.
		if ( AbilitySpec->IsActive() == false )
		{
			continue;
		}

		// 여기서 이제 추가 처리
		// -> 여기서 알아 처리. -> 여기서 추가 처리는 필요할 듯.
		UYSGameplayAbility* TargetAbil = Cast<UYSGameplayAbility>(AbilitySpec->GetPrimaryInstance());

		if ( IsValid(TargetAbil) == false || TargetAbil->IsActive() == false )
			continue;

		if (TargetAbil->TryTransition(InputTag, InputPhase))
			return true;
	}

	return false;
}

void UYSAbilitySystemComponent::StartCoolDown(TSubclassOf<UGameplayAbility> Ability, float TargetToCooldownTime)
{
	FYSCooldownEntry* Entry = CooldownEntries.FindByPredicate([Ability](const FYSCooldownEntry& Entry)
	{
		return Entry.AbilityClass == Ability;
	});
	
	if ( Entry == nullptr )
	{
		CooldownEntries.Add(FYSCooldownEntry{ Ability, TargetToCooldownTime });
		return;
	}
	
	Entry->EndTime = TargetToCooldownTime;
}

bool UYSAbilitySystemComponent::ProcessAbilityByInputPass(const TArray<FGameplayTag>& InputTags, EYSInputPhase InputPhase)
{
	if ( InputTags.Num() == 0 )
	{
		return false;
	}

	// 활성 어빌리티는 태그가 아니라 자기 플레이백 그래프로 판정하므로 세부 태그 한 번이면 된다.
	if ( ProcessAlreadyActiveAbility(InputTags[0], InputPhase) )
		return true;

	if ( InputPhase != EYSInputPhase::Pressed )
		return false;

	// 세부 → 대분류 순. 먼저 맞는 하나만 켠다.
	for ( const FGameplayTag& InputTag : InputTags )
	{
		if ( ProcessSkillActive(InputTag) )
		{
			return true;
		}
	}

	return false;
}


void UYSAbilitySystemComponent::ApplyStatInitialization()
{
	// 교대로 다시 빙의될 때 HP 가 가득 차거나 초기화 GE 가 겹쳐 쌓이지 않게 한다.
	if ( bStatInitialized )
	{
		return;
	}

	AYSCharacterBase* CharacterBase = GetOwner<AYSCharacterBase>();
	
	if ( IsValid(CharacterBase) == false )
	{
		return;
	}

	const FYSCharacterInfo* CharacterInfo = CharacterBase->GetCharacterInfo();
	
	if ( CharacterInfo == nullptr )
	{
		return;
	}

	const UYSDeveloperSettings* DeveloperSettings = GetDefault<UYSDeveloperSettings>();
	if ( DeveloperSettings == nullptr )
	{
		return;
	}

	const FGameplayEffectContextHandle Context = MakeEffectContext();
	const FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(DeveloperSettings->StatInitEffect, 1.f, Context);
	if (!Spec.IsValid())
	{
		return;
	}
	
	const FYSStatBlock& Stats = CharacterInfo->Stats;
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_HP,  Stats.HP);
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_MEL, Stats.MEL);
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_RNG, Stats.RNG);
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_AGI, Stats.AGI);
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_SYN, Stats.SYN);
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_SCL, Stats.SCL);
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_RangeAttackCount, CharacterInfo->MaxRangeAttackCount);

	ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
	bStatInitialized = true;
	ApplyDerivedStats(true);
	ApplyResourceRecharge(CharacterInfo);
}

void UYSAbilitySystemComponent::ApplyResourceRecharge(const FYSCharacterInfo* CharacterInfo)
{
	if (CharacterInfo == nullptr || CharacterInfo->ResourceRechargeEffect == nullptr)
	{
		return;
	}

	// PossessedBy는 교대마다 불린다. 가드가 없으면 충전 GE가 교대 횟수만큼 쌓인다.
	if (ResourceRechargeHandle.IsValid())
	{
		return;
	}

	const FGameplayEffectContextHandle Context = MakeEffectContext();
	const FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(CharacterInfo->ResourceRechargeEffect, 1.f, Context);

	if (Spec.IsValid() == false)
	{
		return;
	}

	ResourceRechargeHandle = ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());
}

void UYSAbilitySystemComponent::ApplyDerivedStats(bool bRefillHp)
{
	const AYSCharacterBase* CharacterBase = GetOwner<AYSCharacterBase>();

	if ( IsValid(CharacterBase) == false )
	{
		return;
	}

	const FYSCharacterInfo* CharacterInfo = CharacterBase->GetCharacterInfo();

	const UYSDeveloperSettings* DeveloperSettings = GetDefault<UYSDeveloperSettings>();
	if ( CharacterInfo == nullptr || DeveloperSettings == nullptr || !DeveloperSettings->StatDeriveEffect )
	{
		return;
	}

	const FGameplayEffectContextHandle Context = MakeEffectContext();
	const FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(DeveloperSettings->StatDeriveEffect, 1.f, Context);

	if (!Spec.IsValid())
	{
		return;
	}

	ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	if (!bRefillHp)
	{
		return;
	}
	
	if (UYSCharacterAttributeSetBase* AttributeSet =
			const_cast<UYSCharacterAttributeSetBase*>(GetSet<UYSCharacterAttributeSetBase>()))
	{
		AttributeSet->SetHp(AttributeSet->GetMaxHp());
		AttributeSet->SetRangeAttackCount(AttributeSet->GetMaxRangeAttackCount());
	}
}

