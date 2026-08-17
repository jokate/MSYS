// Fill out your copyright notice in the Description page of Project Settings.


#include "Details/YSAbilityPlaybackDetails.h"

#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "DetailLayoutBuilder.h"
#include "Playback/YSPlaybackGraphNode.h"

TSharedRef<IDetailCustomization> FYSAbilityPlaybackDetails::MakeInstance()
{
	return MakeShared<FYSAbilityPlaybackDetails>();
}

void FYSAbilityPlaybackDetails::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> CustomizedObjects;
	DetailBuilder.GetObjectsBeingCustomized(CustomizedObjects);

	// 그래프 노드가 소유한 플레이백인지 본다. 하나라도 아니면 건드리지 않는다 —
	// 레거시 배열과 섞여 선택된 경우에 편집 수단을 뺏으면 안 된다.
	bool bOwnedByGraphNode = CustomizedObjects.Num() > 0;

	for (const TWeakObjectPtr<UObject>& WeakObject : CustomizedObjects)
	{
		const UObject* Object = WeakObject.Get();

		if (Object == nullptr || Object->GetOuter() == nullptr || Object->GetOuter()->IsA<UYSPlaybackGraphNode_State>() == false)
		{
			bOwnedByGraphNode = false;
			break;
		}
	}

	if (bOwnedByGraphNode == false)
	{
		return;
	}

	DetailBuilder.HideProperty(GET_MEMBER_NAME_CHECKED(UYSAbilityPlaybackBase, Transitions), UYSAbilityPlaybackBase::StaticClass());
}
