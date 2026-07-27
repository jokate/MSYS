// Fill out your copyright notice in the Description page of Project Settings.


#include "YSAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Ability/YSGameplayAbility.h"
#include "Character/YSCharacterPlayer.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "Data/YSAbilityDataAsset.h"
#include "General/YSGameplayTag.h"
#include "General/YSGeneratedGameplayTags.h"
#include "General/YSStruct.h"
#include "Input/YSInputSaveData.h"
#include "Kismet/GameplayStatics.h"


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
}

void UYSAbilitySystemComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	AYSCharacterPlayer* PC = Cast<AYSCharacterPlayer>(GetOwner());
	if (IsValid(PC))
	{
		if (UYSInputSaveData* SaveGameInstance = Cast<UYSInputSaveData>(UGameplayStatics::CreateSaveGameObject(UYSInputSaveData::StaticClass())))
		{
			for ( FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitySpecHandles )
			{
				FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilitySpecHandle);
			
				if (AbilitySpec == nullptr || AbilitySpec->GetDynamicSpecSourceTags().IsEmpty())
					continue;
			
				FYSInputSaveDataMemeber InputSaveData(AbilitySpec->GetDynamicSpecSourceTags(), AbilitySpec->Ability->GetClass());
				SaveGameInstance->AddInputSaveData(InputSaveData);
			}
		
			UGameplayStatics::SaveGameToSlot(SaveGameInstance,"InputSaveSlot", 0);
		}	
	}
	Super::EndPlay(EndPlayReason);
}

void UYSAbilitySystemComponent::OnGiveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	Super::OnGiveAbility(AbilitySpec);

	// 어빌리티에 대한 등록이 이루어진 경우. 더해주고 빼준다. 차후 해당 데이터를 기반으로 해서 어빌리티를 추적하자.
	AbilitySpecHandles.Emplace(AbilitySpec.Handle);
}

void UYSAbilitySystemComponent::GiveInitAbility()
{
	if ( IsValid(GrantAbilityData) == false )
		return;
	
	
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


void UYSAbilitySystemComponent::OnRemoveAbility(FGameplayAbilitySpec& AbilitySpec)
{
	AbilitySpecHandles.Remove(AbilitySpec.Handle);
	Super::OnRemoveAbility(AbilitySpec);
}

bool UYSAbilitySystemComponent::ProcessSkillActive(const FGameplayTag& InputTag)
{
	for ( FGameplayAbilitySpecHandle& AbilitySpecHandle : AbilitySpecHandles )
	{
		FGameplayAbilitySpec* AbilitySpec = FindAbilitySpecFromHandle(AbilitySpecHandle);

		if ( AbilitySpec == nullptr )
			continue;

        if ( AbilitySpec->IsActive() )
        	continue;
        
		if ( AbilitySpec->GetDynamicSpecSourceTags().HasTagExact(InputTag))
		{
			TryActivateAbility(AbilitySpecHandle);
			return true;
		}
	}

	return false;
}

bool UYSAbilitySystemComponent::ProcessAlreadyActiveAbility(const FGameplayTag& InputTag)
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

		if (TargetAbil->TryTransition(InputTag))
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

void UYSAbilitySystemComponent::ProcessAbilityByInputPass(const FGameplayTag& InputTag)
{
	if ( ProcessAlreadyActiveAbility(InputTag))
		return;

	ProcessSkillActive(InputTag);
}


void UYSAbilitySystemComponent::ApplyStatInitialization()
{
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
	
	if (!CharacterInfo->StatInitEffect)
	{
		return;
	}
 
	const FGameplayEffectContextHandle Context = MakeEffectContext();
	const FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(CharacterInfo->StatInitEffect, 1.f, Context);
	if (!Spec.IsValid())
	{
		return;
	}
 
	// 6스탯을 SetByCaller 로 전달한다.
	// GE_StatInit 은 이 값을 StatXXX 어트리뷰트에 Override 로 그대로 꽂기만 한다.
	// SetByCaller 는 계수/상수 필드가 없어 산술이 불가능하므로, 실수치 환산은 2단계로 분리한다.
	const FYSStatBlock& Stats = CharacterInfo->Stats;
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_HP,  Stats.HP);
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_MEL, Stats.MEL);
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_RNG, Stats.RNG);
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_AGI, Stats.AGI);
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_SYN, Stats.SYN);
	Spec.Data->SetSetByCallerMagnitude(YSTags::Data_Stat_SCL, Stats.SCL);

	ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	// 2단계는 반드시 별도 적용이어야 한다. 같은 Instant GE 안에서 StatXXX 를 쓰고
	// 곧바로 AttributeBased 로 되읽으면 모디파이어 실행 순서와 캡처 타이밍에 의존하게 된다.
	ApplyDerivedStats(/*bRefillHp=*/true);
}

void UYSAbilitySystemComponent::ApplyDerivedStats(bool bRefillHp)
{
	const AYSCharacterBase* CharacterBase = GetOwner<AYSCharacterBase>();

	if ( IsValid(CharacterBase) == false )
	{
		return;
	}

	const FYSCharacterInfo* CharacterInfo = CharacterBase->GetCharacterInfo();

	if ( CharacterInfo == nullptr || !CharacterInfo->StatDeriveEffect )
	{
		return;
	}

	const FGameplayEffectContextHandle Context = MakeEffectContext();
	const FGameplayEffectSpecHandle Spec = MakeOutgoingSpec(CharacterInfo->StatDeriveEffect, 1.f, Context);

	if (!Spec.IsValid())
	{
		return;
	}

	ApplyGameplayEffectSpecToSelf(*Spec.Data.Get());

	if (!bRefillHp)
	{
		return;
	}

	// Hp 를 GE 모디파이어로 채우지 않는 이유 :
	// UYSCharacterAttributeSetBase::AutoRegisterHandler 가 Hp 를 MaxHp 의 CurrentValue 로 클램프한다.
	// MaxHp 갱신보다 Hp 갱신이 먼저 실행되면 생성자 기본값(100)에 잘려버리므로,
	// 파생이 끝난 것이 확정된 이 지점에서 명시적으로 채운다.
	if (UYSCharacterAttributeSetBase* AttributeSet =
			const_cast<UYSCharacterAttributeSetBase*>(GetSet<UYSCharacterAttributeSetBase>()))
	{
		AttributeSet->SetHp(AttributeSet->GetMaxHp());
	}
}

