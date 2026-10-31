// Fill out your copyright notice in the Description page of Project Settings.

#include "Ability/MontageSelector/YSMontageSelector.h"

#include "AbilitySystemComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/MotionWarp/YSMotionWarpParam.h"
#include "Character/YSCharacterBase.h"
#include "Character/Components/YSCharacterMovementComponent.h"
#include "Data/YSAbilityDataAsset.h"
#include "General/YSGameplayTag.h"
#include "MotionWarp/UYSMotionWarpingComponent.h"

void FYSMontageSelector::SetMotionWarp(const UYSGameplayAbility* Ability, bool bInSet) const
{
	if ( IsValid(Ability) == false )
		return;

	AActor* AvatarActor = Ability->GetAvatarActorFromActorInfo();

	if ( IsValid(AvatarActor) == false )
		return;

	const FYSMotionWarpParam* Param = MotionWarpingParam.GetPtr<FYSMotionWarpParam>();
	
	if ( Param == nullptr )
		return;
	
	bInSet ? Param->ResolveMotionWarp(Ability) : Param->ReleaseMotionWarp(Ability);
}

UAnimMontage* FYSMontageSelector_ByDirection::SelectMontage(const UYSGameplayAbility* Ability) const
{
	auto GetMontage = [this](EYSMoveDirection Dir) -> UAnimMontage*
	{
		if (const auto* Entry = DirectionMontages.Find(Dir); Entry && !Entry->IsNull())
			return Entry->LoadSynchronous();

		if (const auto* Fallback = DirectionMontages.Find(EYSMoveDirection::Forward); Fallback && !Fallback->IsNull())
			return Fallback->LoadSynchronous();

		return nullptr;
	};

	return GetMontage(GetMontageDirection(Ability));
}

namespace
{
	// 0 번 칸이 정면 중심에 오도록 반 칸 돌려 시계방향으로 나눈다. 8칸이면 EYSMoveDirection 값과 같다.
	int32 ToSector(const FVector& LocalDirection, int32 SectorCount)
	{
		const float SectorSize      = 360.f / SectorCount;
		const float Angle           = FMath::RadiansToDegrees(FMath::Atan2(LocalDirection.Y, LocalDirection.X));
		const float NormalizedAngle = FMath::Fmod(Angle + SectorSize * 0.5f + 360.f, 360.f);

		return FMath::FloorToInt(NormalizedAngle / SectorSize) % SectorCount;
	}

	FGameplayTag FoldToDirectionTag(const FVector& LocalDirection)
	{
		switch (ToSector(LocalDirection, 4))
		{
		case 1:  return YSTags::Direction_Right;
		case 2:  return YSTags::Direction_Backward;
		case 3:  return YSTags::Direction_Left;
		default: return YSTags::Direction_Forward;
		}
	}

	// 실패하면 미설정을 반환한다 — 방향 태그 없이 조회하고 SelectBest 가 방향 조건 없는 항목으로 폴백한다.
	TOptional<FVector> ResolveLocalDirection(const UYSGameplayAbility* Ability, EYSDirectionSource Source)
	{
		if ( IsValid(Ability) == false )
			return {};

		const AActor* OwningActor = Ability->GetOwningActorFromActorInfo();

		if ( IsValid(OwningActor) == false )
			return {};

		if ( Source == EYSDirectionSource::ControlInput )
		{
			UYSCharacterMovementComponent* Movement = UYSCharacterMovementComponent::Get(Ability->GetAvatarActorFromActorInfo());

			if ( IsValid(Movement) == false )
				return {};

			return Movement->GetLocalSpaceLastInputDirection();
		}

		const FGameplayEventData* EventData = Ability->GetEventData();
		const AActor* Attacker = EventData ? EventData->Instigator.Get() : nullptr;

		if ( IsValid(Attacker) == false )
			return {};

		// 월드 벡터는 내 정면이 반영되지 않으므로 소유자 회전으로 언로테이트해 로컬로 내린다.
		const FVector ToAttacker = (Attacker->GetActorLocation() - OwningActor->GetActorLocation()).GetSafeNormal2D();

		if ( ToAttacker.IsNearlyZero() )
			return {};

		return OwningActor->GetActorRotation().UnrotateVector(ToAttacker);
	}
}

EYSMoveDirection FYSMontageSelector_ControlInputDirection::GetMontageDirection(const UYSGameplayAbility* Ability) const
{
	// 조작감 때문에 입력에 놓인 값을 기준으로 가져온다.
	const TOptional<FVector> LocalInput = ResolveLocalDirection(Ability, EYSDirectionSource::ControlInput);

	return LocalInput ? static_cast<EYSMoveDirection>(ToSector(*LocalInput, 8)) : EYSMoveDirection::Forward;
}

EYSMoveDirection FYSMontageSelector_Target::GetMontageDirection(const UYSGameplayAbility* Ability) const
{
	// 공격자에서 나로 향하는 쪽(밀려나는 쪽)을 내 정면 기준으로 고른다.
	const TOptional<FVector> ToAttacker = ResolveLocalDirection(Ability, EYSDirectionSource::Instigator);

	return ToAttacker ? static_cast<EYSMoveDirection>(ToSector(-*ToAttacker, 8)) : EYSMoveDirection::Forward;
}

FGameplayTagContainer FYSMontageSelector_ByTag::BuildQueryTags(const UYSGameplayAbility* Ability) const
{
	FGameplayTagContainer Query;

	// 공격자가 실어준 히트 강도 등 (Hit.Big / Hit.Normal ...)
	if (IsValid(Ability) == false )
	{
		return Query;
	}
	
	UAbilitySystemComponent* ASC = Ability->GetAbilitySystemComponentFromActorInfo();

	if (IsValid(ASC))
	{
		FGameplayTagContainer TagContainer = ASC->GetOwnedGameplayTags();
		
		FGameplayTagContainer Filtered = TagContainer.Filter(OwnedTagFilter);
		Query.AppendTags(Filtered);
	}
	
	if ( const FGameplayEventData* EventData = Ability->GetEventData() )
		Query.AppendTags(EventData->TargetTags);

	Query.AppendTags(ExtraTags);

	if ( bUseDirectionTag )
	{
		if ( const TOptional<FVector> LocalDirection = ResolveLocalDirection(Ability, DirectionSource) )
			Query.AddTag(FoldToDirectionTag(*LocalDirection));
	}

	return Query;
}

UAnimMontage* FYSMontageSelector_ByTag::SelectMontage(const UYSGameplayAbility* Ability) const
{
	if ( IsValid(Ability) == false )
		return nullptr;

	UYSTaggedMontageAsset* TaggedMontage = UYSTaggedMontageAsset::GetMontageAsset(Ability->GetOwningActorFromActorInfo());

	if ( IsValid(TaggedMontage) == false )
		return nullptr;

	const TSoftObjectPtr<UAnimMontage> MontagePtr = TaggedMontage->SelectBest(BuildQueryTags(Ability));

	if ( MontagePtr.IsNull() )
		return nullptr;

	return MontagePtr.LoadSynchronous();
}
