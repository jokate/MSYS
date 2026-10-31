// Fill out your copyright notice in the Description page of Project Settings.


#include "General/YSStruct.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Ability/Payload/YSAbilityTriggerPayload.h"

EYSTargetHeightBand FYSTargetBandRules::ResolveHeight(float DeltaZ) const
{
	if ( DeltaZ > HeightThreshold )
	{
		return EYSTargetHeightBand::Above;
	}

	return DeltaZ < -HeightThreshold ? EYSTargetHeightBand::Below : EYSTargetHeightBand::Level;
}

EYSTargetDirectionBand FYSTargetBandRules::ResolveDirection(const FVector& Forward, const FVector& ToTarget) const
{
	const FVector Forward2D = Forward.GetSafeNormal2D();
	const FVector ToTarget2D = ToTarget.GetSafeNormal2D();

	if ( Forward2D.IsNearlyZero() || ToTarget2D.IsNearlyZero() )
	{
		return EYSTargetDirectionBand::Front;
	}

	const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(Forward2D, ToTarget2D), -1.f, 1.f)));

	if ( Angle <= FrontAngle )
	{
		return EYSTargetDirectionBand::Front;
	}

	return Angle <= SideAngle ? EYSTargetDirectionBand::Side : EYSTargetDirectionBand::Rear;
}

FGameplayEventData FGameplayEventSendData::MakeEventData(AActor* Instigator, AActor* Target) const
{
	FGameplayEventData EventData;
	EventData.EventTag = TargetToTrigger;
	EventData.Instigator = Instigator;
	EventData.Target = Target;

	// 페이로드 슬롯은 두 개뿐이다.
	if ( TriggerPayloads.IsValidIndex(0) )
	{
		EventData.OptionalObject = TriggerPayloads[0];
	}

	if ( TriggerPayloads.IsValidIndex(1) )
	{
		EventData.OptionalObject2 = TriggerPayloads[1];
	}

	return EventData;
}

void FGameplayEventSendData::SendToActor(AActor* Actor) const
{
	if ( ::IsValid(Actor) == false )
	{
		return;
	}

	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(Actor, TargetToTrigger, MakeEventData(Actor));
}
