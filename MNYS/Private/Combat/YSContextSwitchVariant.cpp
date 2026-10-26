// Fill out your copyright notice in the Description page of Project Settings.


#include "Combat/YSContextSwitchVariant.h"

namespace
{
	bool MaskAccepts(int32 Mask, uint8 Value)
	{
		return Mask == 0 || ( Mask & ( 1 << Value ) ) != 0;
	}
}

EYSContextHeightBand FYSContextSwitchJudgeRules::ResolveHeight(float DeltaZ) const
{
	if ( DeltaZ > HeightThreshold )
	{
		return EYSContextHeightBand::Above;
	}

	return DeltaZ < -HeightThreshold ? EYSContextHeightBand::Below : EYSContextHeightBand::Level;
}

EYSContextDirection FYSContextSwitchJudgeRules::ResolveDirection(const FVector& Forward, const FVector& ToTarget) const
{
	const FVector Forward2D = Forward.GetSafeNormal2D();
	const FVector ToTarget2D = ToTarget.GetSafeNormal2D();

	if ( Forward2D.IsNearlyZero() || ToTarget2D.IsNearlyZero() )
	{
		return EYSContextDirection::Front;
	}

	const float Angle = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(FVector::DotProduct(Forward2D, ToTarget2D), -1.f, 1.f)));

	if ( Angle <= FrontAngle )
	{
		return EYSContextDirection::Front;
	}

	return Angle <= SideAngle ? EYSContextDirection::Side : EYSContextDirection::Rear;
}

bool FYSContextSwitchVariant::Matches(const FYSContextSwitchSituation& Situation) const
{
	if ( TargetRequiredTags.IsEmpty() == false && Situation.TargetTags.HasAll(TargetRequiredTags) == false )
	{
		return false;
	}

	if ( Situation.TargetTags.HasAny(TargetExcludedTags) )
	{
		return false;
	}

	return MaskAccepts(HeightMask, static_cast<uint8>(Situation.Height))
		&& MaskAccepts(DistanceMask, static_cast<uint8>(Situation.Distance))
		&& MaskAccepts(DirectionMask, static_cast<uint8>(Situation.Direction));
}

int32 FYSContextSwitchVariant::GetSpecificity() const
{
	const bool bState = TargetRequiredTags.IsEmpty() == false || TargetExcludedTags.IsEmpty() == false;

	return ( bState ? 8 : 0 ) + ( HeightMask != 0 ? 4 : 0 ) + ( DistanceMask != 0 ? 2 : 0 ) + ( DirectionMask != 0 ? 1 : 0 );
}

FName FYSContextSwitchVariant::FindBestRow(const UDataTable* Table, TFunctionRef<bool(const FYSContextSwitchVariant&)> InputMatcher, const FYSContextSwitchSituation& Situation)
{
	if ( IsValid(Table) == false || Table->GetRowStruct() == nullptr || Table->GetRowStruct()->IsChildOf(StaticStruct()) == false )
	{
		return NAME_None;
	}

	FName Best = NAME_None;
	int32 BestSpecificity = -1;

	// 같은 구체성이면 표의 앞 행.
	for ( const TPair<FName, uint8*>& Pair : Table->GetRowMap() )
	{
		const FYSContextSwitchVariant* Variant = reinterpret_cast<const FYSContextSwitchVariant*>(Pair.Value);

		if ( InputMatcher(*Variant) == false || Variant->Matches(Situation) == false )
		{
			continue;
		}

		const int32 Specificity = Variant->GetSpecificity();

		if ( Specificity > BestSpecificity )
		{
			Best = Pair.Key;
			BestSpecificity = Specificity;
		}
	}

	return Best;
}
