
#pragma once

#include "IDetailCustomization.h"
#include "DetailLayoutBuilder.h"

class AYSCharacterBase;
class AYSAbilityTestCharacter;
class UYSAbilityDataAsset;
class UGameplayAbility;

class FYSAbilityTestCharacterDetails : public IDetailCustomization
{
public:
	static TSharedRef<IDetailCustomization> MakeInstance();

	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	void GenerateAbilityButtons(IDetailCategoryBuilder& CategoryBuilder, const AYSCharacterBase* TestCharacter);
    
	FReply OnAbilityButtonClicked(TWeakObjectPtr<AYSCharacterBase> TestCharacter, TSubclassOf<UGameplayAbility> AbilityClass);
};