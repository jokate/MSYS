// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/Ability/YSGameplayAbility_Hit.h"

#include "AbilitySystemComponent.h"
#include "YSBattleActor.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"

void UYSGameplayAbility_Hit::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                             const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                             const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);
	
	_ProcessDamage(TriggerEventData);
}

bool UYSGameplayAbility_Hit::CanActivateAbility(const FGameplayAbilitySpecHandle Handle,
	const FGameplayAbilityActorInfo* ActorInfo, const FGameplayTagContainer* SourceTags,
	const FGameplayTagContainer* TargetTags, FGameplayTagContainer* OptionalRelevantTags) const
{
	if ( IYSBattleActor* BattleActor = Cast<IYSBattleActor>(ActorInfo->OwnerActor) )
	{
		if ( BattleActor->IsDead() )
			return false;
	}
	return Super::CanActivateAbility(Handle, ActorInfo, SourceTags, TargetTags, OptionalRelevantTags);
}

void UYSGameplayAbility_Hit::_ProcessDamage(const FGameplayEventData* TriggerData)
{
	if ( TriggerData == nullptr )
	{
		return;
	}
	
	UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
	const UYSCharacterAttributeSetBase* TargetAttributeSet = ASC->GetSet<UYSCharacterAttributeSetBase>();

	if ( IsValid(TargetAttributeSet) == false )
		return;
		
	float CurHp = TargetAttributeSet->GetHp();
		
	if ( CurHp <= 0.f )
		return;
	
	if ( IsValid(ASC) )
	{
		// 데미지 처리 로직 추가. ( 클램핑은 내부에서 알아 처리 될 거임 )
		ASC->SetNumericAttributeBase(TargetAttributeSet->GetHpAttribute(), CurHp - TriggerData->EventMagnitude);		
	}
}
