
#include "Details/YSAbilityTestCharacterDetails.h"
#include "Data/YSAbilityDataAsset.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SExpandableArea.h"
#include "AbilitySystemComponent.h"
#include "DetailWidgetRow.h"
#include "YSAbilitySystemComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "Character/YSCharacterBase.h"

TSharedRef<IDetailCustomization> FYSAbilityTestCharacterDetails::MakeInstance()
{
    return MakeShared<FYSAbilityTestCharacterDetails>();
}

void FYSAbilityTestCharacterDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
    TArray<TWeakObjectPtr<UObject>> Objects;
    DetailBuilder.GetObjectsBeingCustomized(Objects);

    if (Objects.Num() == 0) return;

    AYSCharacterBase* TestCharacter = Cast<AYSCharacterBase>(Objects[0].Get());
    if (!TestCharacter) return;

    // 기본 프로퍼티 표시
    DetailBuilder.EditCategory("YS | Test")
        .SetSortOrder(0);

    // 커스텀 카테고리 추가
    IDetailCategoryBuilder& AbilityCategory = DetailBuilder.EditCategory("YS | Abilities", FText::FromString("Abilities"), ECategoryPriority::Important);
    
    GenerateAbilityButtons(AbilityCategory, TestCharacter);
}

void FYSAbilityTestCharacterDetails::GenerateAbilityButtons(IDetailCategoryBuilder& CategoryBuilder,
    const AYSCharacterBase* TestCharacter)
{
    if (IsValid(TestCharacter) == false ) 
        return;
    
    UYSAbilitySystemComponent* YSAbil = Cast<UYSAbilitySystemComponent>(TestCharacter->GetAbilitySystemComponent());
   
    if ( IsValid(YSAbil) == false )
        return;
    
    UYSAbilityDataAsset* GrantAbilAsset = YSAbil->GrantAbilityData;
    
    if ( IsValid(GrantAbilAsset) == false )
        return;
    
    TArray<FYSGrantedAbilityData> AllAbilities = GrantAbilAsset->GetAllAbilities();

    for (const FYSGrantedAbilityData& AbilityData : AllAbilities)
    {
        if (!AbilityData.AbilityClass) continue;

        TSubclassOf<UGameplayAbility> AbilityClassCopy = AbilityData.AbilityClass;
        TWeakObjectPtr<const AYSCharacterBase> CharacterPtr(TestCharacter);

        CategoryBuilder.AddCustomRow(FText::FromString(AbilityData.AbilityClass->GetName()))
            [
                SNew(SButton)
                .Text(FText::FromString(AbilityData.AbilityClass->GetName()))
                .OnClicked_Lambda([this, CharacterPtr, AbilityClassCopy]()
                {
                    return OnAbilityButtonClicked(TWeakObjectPtr<AYSCharacterBase>(const_cast<AYSCharacterBase*>(CharacterPtr.Get())), AbilityClassCopy);
                })
            ];
    }
}

FReply FYSAbilityTestCharacterDetails::OnAbilityButtonClicked(TWeakObjectPtr<AYSCharacterBase> TestCharacter,
                                                              TSubclassOf<UGameplayAbility> AbilityClass)
{
    AYSCharacterBase* Character = TestCharacter.Get();
    if (!Character || !AbilityClass) return FReply::Handled();

    if (Character->AbilitySystemComponent)
    {
        Character->AbilitySystemComponent->TryActivateAbilityByClass(AbilityClass);
    }

    return FReply::Handled();
}