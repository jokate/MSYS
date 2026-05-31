// Fill out your copyright notice in the Description page of Project Settings.


#include "Library/YSBlueprintFunctionLibrary.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "YSDeveloperSettings.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Character/YSCharacterBase.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "Character/Components/YSLockOnComponent.h"
#include "General/YSGameplayTag.h"

float UYSBlueprintFunctionLibrary::GetFinalDamage(const UYSCharacterAttributeSetBase* Owner,
                                                  const UYSCharacterAttributeSetBase* Target, const FName& SkillID)
{
	const FYSDamageInfo* DamageInfo = UYSDeveloperSettings::GetDamageInfo(SkillID);
	
	if (IsValid(Owner) == false || IsValid(Target) == false )
		return 0.f;

	// 데미지 정보가 없으면 의미 X
	if ( DamageInfo == nullptr )
		return 0.f;
	
	// 당장 생각하는 부분은 (공격 데미지 * (공격 계수)  ) * ( 1 - DefenceRate ) + 고정 데미지
	float FinalDamage = ( Owner->GetAttackDmg() * DamageInfo->DamageMultiplier ) * ( 1 - Target->GetDefenseRate()) + DamageInfo->AdditiveTrueDamage;

	// 차후 데미지 증가 버프 혹은 다른 것들이 추가될 경우 해당 부분에 대해서 확장하기로 합니다.
	
	return FinalDamage;
}

void UYSBlueprintFunctionLibrary::SendHitEventToTarget(AActor* Instigator, AActor* Target, const FName& SkillID)
{
	const FYSDamageInfo* DamageInfo = UYSDeveloperSettings::GetDamageInfo(SkillID);

	// 데미지 정보가 없으면 의미 X
	if ( DamageInfo == nullptr )
		return;

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);
	UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);
	
	if ( IsValid(ASC) == false || IsValid(TargetASC) == false )
		return;
	
	float FinalDamage = GetFinalDamage(ASC->GetSet<UYSCharacterAttributeSetBase>(), ASC->GetSet<UYSCharacterAttributeSetBase>(), SkillID);
	
	FGameplayEventData EventData;
	EventData.Instigator = Instigator;
	EventData.Target = Target;
	EventData.EventMagnitude = FinalDamage;

	TargetASC->HandleGameplayEvent(DamageInfo->HitTag, &EventData);
}

FRotator UYSBlueprintFunctionLibrary::GetAbilityEventRotation(EYSDirectionPolicy DirectionPolicy,
	UYSGameplayAbility* OwningAbility, const FName& SocketName)
{
	AActor* OwnerActor = OwningAbility->GetOwningActorFromActorInfo();
	
	if ( IsValid(OwnerActor) == false )
	{
		return FRotator();
	}
	
	switch (DirectionPolicy)
	{
	case EYSDirectionPolicy::UseSocketRotation:
		{
			AYSCharacterBase* Character = Cast<AYSCharacterBase>(OwnerActor);
			if (IsValid(Character) && IsValid(Character->GetMesh()))
			{
				if (SocketName != NAME_None)
					return Character->GetMesh()->GetSocketRotation(SocketName);
			}
			return OwnerActor->GetActorRotation();
		}

	case EYSDirectionPolicy::UseControlRotation:
		{
			if (const AController* Controller = OwnerActor->GetInstigatorController())
				return Controller->GetControlRotation();

			return OwnerActor->GetActorRotation();
		}

	case EYSDirectionPolicy::UseTowardLockOnTarget:
		{
			if (UYSLockOnComponent* LockOn = UYSLockOnComponent::Get(OwnerActor))
			{
				if (AActor* Target = LockOn->GetCurrentTarget())
				{
					const FVector Dir = (Target->GetActorLocation() - OwnerActor->GetActorLocation()).GetSafeNormal();
					return Dir.Rotation();
				}
			}
			return OwnerActor->GetActorRotation();
		}

	case EYSDirectionPolicy::UseTowardPlaybackTarget:
		{
			if (const UYSAbilityPlaybackBase* Playback = OwningAbility->GetCurrentPlayback())
			{
				if (AActor* Target = Playback->GetCurrentPlaybackTarget())
				{
					const FVector Dir = (Target->GetActorLocation() - OwnerActor->GetActorLocation()).GetSafeNormal();
					return Dir.Rotation();
				}
			}
			return OwnerActor->GetActorRotation();
		}

	case EYSDirectionPolicy::UseActorForwardVector:
	default:
		return OwnerActor->GetActorRotation();
	}
}

FVector UYSBlueprintFunctionLibrary::GetAbilityEventPosition(EYSPositionPolicy PositionPolicy,
	UYSGameplayAbility* OwningAbility, const FName& SocketName, const FVector& RelativeOffset)
{
	AActor* OwnerActor = OwningAbility->GetOwningActorFromActorInfo();
	
	if ( IsValid(OwnerActor) == false )
	{
		return FVector::ZeroVector;
	}
	
	switch (PositionPolicy)
	{
	case EYSPositionPolicy::UseSocket:
		{
			AYSCharacterBase* Character = Cast<AYSCharacterBase>(OwnerActor);
			if (IsValid(Character) && IsValid(Character->GetMesh()) && SocketName != NAME_None)
			{
				return Character->GetMesh()->GetSocketLocation(SocketName);
			}
			return OwnerActor->GetActorLocation();
		}

	case EYSPositionPolicy::UseRelativeOffset:
		return OwnerActor->GetActorTransform().TransformPosition(RelativeOffset);

	case EYSPositionPolicy::UseActorLocation:
	default:
		return OwnerActor->GetActorLocation();
	}
}
