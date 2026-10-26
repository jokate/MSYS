// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/AttributeSet/YSGroggyAttributeSet.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "GameplayEffectExtension.h"
#include "Ability/GameplayEffect/YSGE_GroggyDamage.h"
#include "Combat/YSGroggyComponent.h"
#include "Combat/YSGroggyRules.h"
#include "General/YSGameplayTag.h"

UYSGroggyAttributeSet::UYSGroggyAttributeSet()
{
	// 임시값. 적마다 다른 값은 초기화 GE 로 덮는다.
	InitGroggy(100.f);
	InitMaxGroggy(100.f);
	InitIncomingGroggyDamage(0.f);
}

bool UYSGroggyAttributeSet::RestoreGroggy(AActor* Target, float Ratio)
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	if ( IsValid(ASC) == false || ASC->HasAttributeSetForAttribute(GetGroggyAttribute()) == false )
	{
		return false;
	}

	const float Max = ASC->GetNumericAttribute(GetMaxGroggyAttribute());
	ASC->SetNumericAttributeBase(GetGroggyAttribute(), Max * FMath::Clamp(Ratio, 0.f, 1.f));
	return true;
}

bool UYSGroggyAttributeSet::ApplyGroggyDamage(AActor* Instigator, AActor* Target, float Amount)
{
	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

	if ( IsValid(SourceASC) == false || IsValid(TargetASC) == false || TargetASC->HasAttributeSetForAttribute(GetGroggyAttribute()) == false )
	{
		return false;
	}

	const UYSGroggyComponent* GroggyComponent = Target->FindComponentByClass<UYSGroggyComponent>();
	const float Multiplier = IsValid(GroggyComponent) ? GroggyComponent->GetRules().GroggyDamageTakenMultiplier : 1.f;

	FGameplayEffectContextHandle Context = SourceASC->MakeEffectContext();
	Context.AddInstigator(Instigator, Instigator);

	FGameplayEffectSpec Spec(GetDefault<UYSGE_GroggyDamage>(), Context);
	Spec.SetSetByCallerMagnitude(UYSGE_GroggyDamage::GroggyDamageName, Amount * Multiplier);

	SourceASC->ApplyGameplayEffectSpecToTarget(Spec, TargetASC);
	return true;
}

void UYSGroggyAttributeSet::LockDamage(float Seconds)
{
	DamageLockedUntil = GetWorldTime() + FMath::Max(Seconds, 0.f);
}

double UYSGroggyAttributeSet::GetWorldTime() const
{
	const UWorld* World = GetWorld();
	return World != nullptr ? World->GetTimeSeconds() : 0.0;
}

void UYSGroggyAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);

	if ( Attribute == GetGroggyAttribute() )
	{
		NewValue = FMath::Clamp(NewValue, 0.f, GetMaxGroggy());
	}
}

void UYSGroggyAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if ( Data.EvaluatedData.Attribute != GetIncomingGroggyDamageAttribute() )
	{
		return;
	}

	const float Damage = GetIncomingGroggyDamage();
	SetIncomingGroggyDamage(0.f);

	const double Now = GetWorldTime();

	if ( Damage <= 0.f || Now < DamageLockedUntil )
	{
		return;
	}

	LastDamageTime = Now;

	const float OldGroggy = GetGroggy();
	SetGroggy(FMath::Clamp(OldGroggy - Damage, 0.f, GetMaxGroggy()));

	// 0 에 "닿는 순간" 한 번만. 이미 0 인 채로 더 맞아도 다시 보내지 않는다.
	if ( OldGroggy > 0.f && GetGroggy() <= 0.f )
	{
		AActor* Owner = GetOwningActor();

		FGameplayEventData Payload;
		Payload.EventTag = YSTags::Event_GroggyBroken;
		Payload.Instigator = Data.EffectSpec.GetContext().GetInstigator();
		Payload.Target = Owner;

		UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Owner, YSTags::Event_GroggyBroken, Payload);
	}
}
