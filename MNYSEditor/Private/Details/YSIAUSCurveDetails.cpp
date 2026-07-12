// Fill out your copyright notice in the Description page of Project Settings.


#include "Details/YSIAUSCurveDetails.h"

#include "DetailCategoryBuilder.h"
#include "DetailLayoutBuilder.h"
#include "DetailWidgetRow.h"
#include "IPropertyUtilities.h"
#include "Ability/AIAbility/YSAIAbilityScoreFunction.h"
#include "Ability/AIAbility/YSGameplayAbility_AIBase.h"
#include "Details/YSIAUSCurvePreviewWidget.h"
#include "Widgets/Text/STextBlock.h"

#define LOCTEXT_NAMESPACE "YSIAUSCurveDetails"

TSharedRef<IDetailCustomization> FYSIAUSCurveDetails::MakeInstance()
{
	return MakeShareable(new FYSIAUSCurveDetails);
}

void FYSIAUSCurveDetails::CustomizeDetails(IDetailLayoutBuilder& DetailLayout)
{
	TArray<TWeakObjectPtr<UObject>> EditedObjects;
	DetailLayout.GetObjectsBeingCustomized(EditedObjects);

	UYSGameplayAbility_AIBase* Ability = nullptr;
	for (const TWeakObjectPtr<UObject>& EditedObject : EditedObjects)
	{
		Ability = Cast<UYSGameplayAbility_AIBase>(EditedObject.Get());
		if (Ability)
		{
			break;
		}
	}

	if (!Ability)
	{
		return;
	}
	
	const TSharedRef<IPropertyUtilities> PropertyUtilities = DetailLayout.GetPropertyUtilities();
	const FSimpleDelegate RefreshDelegate = FSimpleDelegate::CreateSP(PropertyUtilities, &IPropertyUtilities::ForceRefresh);

	const TSharedRef<IPropertyHandle> UtilityScoreHandle = DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UYSGameplayAbility_AIBase, UtilityScore));
	UtilityScoreHandle->SetOnPropertyValueChanged(RefreshDelegate);

	uint32 NumElements = 0;
	UtilityScoreHandle->GetNumChildren(NumElements);
	for (uint32 ElementIndex = 0; ElementIndex < NumElements; ++ElementIndex)
	{
		if (const TSharedPtr<IPropertyHandle> ElementHandle = UtilityScoreHandle->GetChildHandle(ElementIndex))
		{
			ElementHandle->SetOnPropertyValueChanged(RefreshDelegate);
		}
	}

	// UtilityScore가 속한 카테고리(YS | Condition) 안에서 배열 바로 아래에 프리뷰를 끼워 넣는다.
	// AddProperty로 순서를 명시하면 기본 배치 대신 추가한 순서대로 나열된다.
	IDetailCategoryBuilder& ConditionCategory = DetailLayout.EditCategory(UtilityScoreHandle->GetDefaultCategoryName());
	ConditionCategory.AddProperty(UtilityScoreHandle);

	PreviewWidgets.Reset();
	for (int32 Index = 0; Index < Ability->UtilityScore.Num(); ++Index)
	{
		UYSAIAbilityScoreFunctionBase* ScoreFunction = Ability->UtilityScore[Index];
		if (!ScoreFunction)
		{
			continue;
		}

		TSharedPtr<SResponseCurvePreviewWidget> PreviewWidget;
		ConditionCategory.AddCustomRow(LOCTEXT("PreviewRowFilter", "Preview"))
			.NameContent()
			[
				SNew(STextBlock)
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.Text(FText::Format(LOCTEXT("PreviewRowName", "[{0}] {1}"), Index, ScoreFunction->GetClass()->GetDisplayNameText()))
			]
			.ValueContent()
			[
				SAssignNew(PreviewWidget, SResponseCurvePreviewWidget)
			];

		PreviewWidget->ResponseCurve = ScoreFunction;
		PreviewWidgets.Add(PreviewWidget);
	}

	// 프리뷰 행 뒤에 오도록 나머지 프로퍼티를 원래 선언 순서대로 다시 추가한다
	ConditionCategory.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UYSGameplayAbility_AIBase, BaseUtilityScore)));
	ConditionCategory.AddProperty(DetailLayout.GetProperty(GET_MEMBER_NAME_CHECKED(UYSGameplayAbility_AIBase, UtilityScoreThreshold)));
}

#undef LOCTEXT_NAMESPACE
