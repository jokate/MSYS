// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"

#include "GameplayEffectExtension.h"
#include "Character/YSPlayerController.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "General/YSGameplayTag.h"
#include "Subsystem/YSSquadSubsystem.h"

UYSCharacterAttributeSetBase::UYSCharacterAttributeSetBase()
{
	InitStatHP(5.f);
	InitStatMEL(5.f);
	InitStatRNG(5.f);
	InitStatAGI(5.f);
	InitStatSYN(5.f);
	InitStatSCL(5.f);
	
	InitHp(100.f);
	InitMaxHp(100.f);
	InitMeleeAttackDmg(10.f);
	InitRangedAttackDmg(10.f);
	InitDefenseRate(0.1f);
	InitMoveSpeed(600.f);
	InitTagGaugeRate(1.f);
	InitIncomingDamage(0.f);
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
	
	if ( Attribute == GetIncomingDamageAttribute() )
	{
		const float Damage = GetIncomingDamage();
		SetIncomingDamage(0.f); // 메타는 즉시 비운다
 
		if (Damage <= 0.f)
		{
			return;
		}
 
		UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();
 
		// 무적 상태면 데미지를 통째로 버린다.
		// 저스트 회피의 완전 무적이 여기서 성립한다.
		if (TargetASC && TargetASC->HasMatchingGameplayTag(YSTags::Invincible))
		{
			return;
		}
		
		SetHp(FMath::Clamp(GetHp() - Damage, 0.f, GetMaxHp()));
		return;
	}
	
	if ( Attribute == GetHpAttribute() )
	{ 
		if (NewValue <= 0.f && OldValue > 0.f)
		{	
			FGameplayEventData EventData;
			HandleDowned();
			GetOwningAbilitySystemComponent()->HandleGameplayEvent(YSTags::Event_OnDead, &EventData);
		}
		
		return;
	}
}

void UYSCharacterAttributeSetBase::PostGameplayEffectExecute(const struct FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);
	
	if (Data.EvaluatedData.Attribute == GetMoveSpeedAttribute())
	{
		if (ACharacter* Character = Cast<ACharacter>(GetOwningActor()))
		{
			if (UCharacterMovementComponent* Movement = Character->GetCharacterMovement())
			{
				Movement->MaxWalkSpeed = GetMoveSpeed();
			}
		}
		return;
	}
}

void UYSCharacterAttributeSetBase::HandleDowned()
{
	AActor* Owner = GetOwningActor();
	if (IsValid(Owner) == false)
	{
		return;
	}

	AYSPlayerController* PlayerController = Cast<AYSPlayerController>(Owner->GetInstigatorController());
	
	if ( IsValid(PlayerController) == false )
	{
		return;
	}
	
	OnCharacterDowned.Broadcast(Owner);
 
	UGameInstance* GI = Owner->GetGameInstance();

	if ( IsValid(GI) == false )
	{
		return;
	}
	
	if (UYSSquadSubsystem* Squad = GI->GetSubsystem<UYSSquadSubsystem>())
	{
		Squad->ForceTagOnDown();
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
