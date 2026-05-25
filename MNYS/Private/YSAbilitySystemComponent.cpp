// Fill out your copyright notice in the Description page of Project Settings.


#include "YSAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Ability/YSGameplayAbility.h"
#include "Character/YSCharacterPlayer.h"
#include "Data/YSAbilityDataAsset.h"
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


void UYSAbilitySystemComponent::ProcessAbilityByInputPass(const FGameplayTag& InputTag)
{
	if ( ProcessAlreadyActiveAbility(InputTag))
		return;

	ProcessSkillActive(InputTag);
}


