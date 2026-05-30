// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"

#include "General/YSGameplayTag.h"

UYSCharacterAttributeSetBase::UYSCharacterAttributeSetBase()
{
	Hp = 100.f;
	MaxHp = 100.f;
	AttackDmg = 10.f;
	DefenseRate = 0.1f;
}

void UYSCharacterAttributeSetBase::PostInitProperties()
{
	Super::PostInitProperties();
	
	AutoRegisterHandler();
}

void UYSCharacterAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
    if (const FClampHandler* Handler = ClampRegistry.Find(Attribute))
    {
        (*Handler)(NewValue);
    }
}

void UYSCharacterAttributeSetBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue,
	float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if ( Attribute == GetHpAttribute() )
	{ 
		if (NewValue <= 0.f && OldValue > 0.f)
		{	
			FGameplayEventData EventData;
			GetOwningAbilitySystemComponent()->HandleGameplayEvent(YSTags::Event_OnDead, &EventData);
		}
	}
}

void UYSCharacterAttributeSetBase::AutoRegisterHandler()
{
	for (TFieldIterator<FStructProperty> It(GetClass()); It; ++It)
	{
		FStructProperty* Prop = *It;
		// FGameplayAttributeData 타입만 처리
		if (Prop->Struct != FGameplayAttributeData::StaticStruct())
		{
			continue;
		}
		
		// Max 속성 자체는 클램핑 대상이 아님
		if (Prop->GetName().StartsWith(TEXT("Max")))
		{
			continue;
		}
		
		// 네이밍 컨벤션으로 Max 대응 속성 탐색: Health → MaxHealth
		const FString MaxPropName = TEXT("Max") + Prop->GetName();
		FStructProperty* MaxProp = FindFProperty<FStructProperty>(GetClass(), *MaxPropName);
		if (!MaxProp || MaxProp->Struct != FGameplayAttributeData::StaticStruct())
		{
			continue;
		}
		// 실제 메모리 주소를 캡처 — AttributeSet과 생명주기가 같으므로 안전
		FGameplayAttributeData* MaxData = MaxProp->ContainerPtrToValuePtr<FGameplayAttributeData>(this);
		ClampRegistry.Add(FGameplayAttribute(Prop), [MaxData](float& V)
		{
			V = FMath::Clamp(V, 0.f, MaxData->GetCurrentValue());
		});
	}
}
