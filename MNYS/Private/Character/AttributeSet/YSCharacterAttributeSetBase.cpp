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
	InitIncomingDamageMultiplier(1.f);
}

void UYSCharacterAttributeSetBase::PostInitProperties()
{
	Super::PostInitProperties();
	
	AutoRegisterHandler();
}

void UYSCharacterAttributeSetBase::PreAttributeBaseChange(const FGameplayAttribute& Attribute, float& NewValue) const
{
	Super::PreAttributeBaseChange(Attribute, NewValue);
	
	ApplyAttributeGuards(Attribute, NewValue);
}

void UYSCharacterAttributeSetBase::ApplyAttributeGuards(const FGameplayAttribute& Attribute, float& NewValue) const
{
	CallHandler(Attribute, NewValue);

	if (Attribute == GetIncomingDamageMultiplierAttribute())
	{
		NewValue = FMath::Max(NewValue, 0.f);
	}

	// 피해 식이 (1 - DefenseRate) 이므로 1.0 이 완전 방어다. 넘으면 피해가 음수(회복)가 된다.
	if (Attribute == GetDefenseRateAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.f, 1.f);
	}
}

void UYSCharacterAttributeSetBase::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
	
	ApplyAttributeGuards(Attribute, NewValue);
}

void UYSCharacterAttributeSetBase::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue,
	float NewValue)
{
	Super::PostAttributeChange(Attribute, OldValue, NewValue);
	
	if ( Attribute == GetIncomingDamageAttribute() )
	{
		const float Damage = GetIncomingDamage();
		IncomingDamage = 0.f;
		
		if (Damage <= 0.f)
		{
			return;
		}
 
		UAbilitySystemComponent* TargetASC = GetOwningAbilitySystemComponent();
 
		// 무적 상태면 데미지를 통째로 버린다 (저스트 회피의 완전 무적이 여기서 성립한다).
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
			UAbilitySystemComponent* DeadASC = GetOwningAbilitySystemComponent();
			HandleDowned();
			if (IsValid(DeadASC))
			{
				DeadASC->HandleGameplayEvent(YSTags::Event_OnDead, &EventData);
			}
		}
		
		return;
	}
	
	// 따로 특정 이벤트를 발동할 게 아니라 그냥 태그를 붙이거나 제거하는 용도라면 PostAttributeChange에서 처리한다.
	if (Attribute == GetRangeAttackCountAttribute())
	{
		UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
		if (IsValid(ASC) == false)
		{
			return;
		}
		
		ASC->SetLooseGameplayTagCount(YSTags::State_Resource_Empty, NewValue <= 0.f ? 1 : 0);
		ASC->SetLooseGameplayTagCount(YSTags::State_Resource_NotFull, NewValue < GetMaxRangeAttackCount() ? 1 : 0);
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
		// 쓰러진 폰에 창이 남으면 벤치로 들어가서도 만료까지 열려 있다.
		Squad->CloseSwitchWindow(GetOwningAbilitySystemComponent());
		Squad->ForceTagOnDown();
	}	
}

void UYSCharacterAttributeSetBase::AutoRegisterHandler()
{
	for (TFieldIterator<FStructProperty> It(GetClass()); It; ++It)
	{
		FStructProperty* Prop = *It;
		if (Prop->Struct != FGameplayAttributeData::StaticStruct())
		{
			continue;
		}
		
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

void UYSCharacterAttributeSetBase::CallHandler(const FGameplayAttribute& Attribute, float& NewValue) const
{
	if (const FClampHandler* Handler = ClampRegistry.Find(Attribute))
	{
		(*Handler)(NewValue);
	}
}
