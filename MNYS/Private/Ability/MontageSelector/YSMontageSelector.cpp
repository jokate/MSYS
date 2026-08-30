// Fill out your copyright notice in the Description page of Project Settings.

#include "Ability/MontageSelector/YSMontageSelector.h"
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

// 다 좋은데 입력 방
UAnimMontage* FYSMontageSelector_ByDirection::SelectMontage(const UYSGameplayAbility* Ability) const
{
	auto GetMontage = [this](EYSMoveDirection Dir) -> UAnimMontage*
	{
		if (const auto* Entry = DirectionMontages.Find(Dir); Entry && !Entry->IsNull())
			return Entry->LoadSynchronous();

		// 해당 방향 미등록 시 Forward 폴백
		if (const auto* Fallback = DirectionMontages.Find(EYSMoveDirection::Forward); Fallback && !Fallback->IsNull())
			return Fallback->LoadSynchronous();

		return nullptr;
	};

	return GetMontage(GetMontageDirection(Ability));
}

EYSMoveDirection FYSMontageSelector_ControlInputDirection::GetMontageDirection(const UYSGameplayAbility* Ability) const
{
	// 조작감 때문에 입력에 놓여진 값들을 기준으로 가져오도록 합시다
	AActor* AvatarActor = Ability->GetAvatarActorFromActorInfo();
	UYSCharacterMovementComponent* Movement = UYSCharacterMovementComponent::Get(AvatarActor);
	
	if (!IsValid(Movement))
		return EYSMoveDirection::Forward;

	// 로컬 스페이스 정규화 벡터 (입력 없으면 ForwardVector 유지)
	const FVector LocalInput = Movement->GetLocalSpaceLastInputDirection();

	// atan2(Y, X): Y = 로컬 우측, X = 로컬 전방  →  [-180, 180]
	const float Angle           = FMath::RadiansToDegrees(FMath::Atan2(LocalInput.Y, LocalInput.X));
	const float NormalizedAngle = FMath::Fmod(Angle + 22.5f + 360.f, 360.f);
	const int32 SectorIndex     = FMath::FloorToInt(NormalizedAngle / 45.f) % 8;
	
	return static_cast<EYSMoveDirection>(SectorIndex);
}

EYSMoveDirection FYSMontageSelector_Target::GetMontageDirection(const UYSGameplayAbility* Ability) const
{
	if ( IsValid(Ability) == false )
		return EYSMoveDirection::Forward;
	
	const FGameplayEventData* EventData = Ability->GetEventData();
	
	if (EventData == nullptr )
		return EYSMoveDirection::Forward;
	
	const AActor* TargetActor = EventData->Instigator;
	AActor* OwningActor = Ability->GetOwningActorFromActorInfo();
	if ( IsValid(TargetActor) == false )
		return EYSMoveDirection::Forward;
	
	const FVector FromAttacker = ((OwningActor->GetActorLocation() - TargetActor->GetActorLocation()).GetSafeNormal2D());
	const float Angle           = FMath::RadiansToDegrees(FMath::Atan2(FromAttacker.Y, FromAttacker.X));
	const float NormalizedAngle = FMath::Fmod(Angle + 22.5f + 360.f, 360.f);
	const int32 SectorIndex     = FMath::FloorToInt(NormalizedAngle / 45.f) % 8;
	
	return static_cast<EYSMoveDirection>(SectorIndex);
}

namespace
{
	// 로컬 스페이스 XY 방향을 4방향 태그로 접는다.
	// 90° 섹터를 45° 오프셋해서, 각 방향이 섹터 중심에 오도록 정렬한다.
	//   Forward [-45,45)  Right [45,135)  Backward [135,225)  Left [225,315)
	FGameplayTag FoldToDirectionTag(const FVector& LocalDirection)
	{
		const float Angle           = FMath::RadiansToDegrees(FMath::Atan2(LocalDirection.Y, LocalDirection.X));
		const float NormalizedAngle = FMath::Fmod(Angle + 45.f + 360.f, 360.f);

		switch (FMath::FloorToInt(NormalizedAngle / 90.f) % 4)
		{
		case 1:  return YSTags::Direction_Right;
		case 2:  return YSTags::Direction_Backward;
		case 3:  return YSTags::Direction_Left;
		default: return YSTags::Direction_Forward;
		}
	}

	// 방향 산출에 실패하면 미설정을 반환한다. 이때는 방향 태그 없이 조회하고,
	// SelectBest 가 방향 조건이 없는 항목으로 알아서 폴백한다.
	TOptional<FVector> ResolveLocalDirection(const UYSGameplayAbility* Ability, EYSDirectionSource Source)
	{
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

		// 공격자가 "내 기준" 어느 쪽에 있는가. 월드 벡터를 그대로 쓰면 내 정면이 반영되지 않으므로
		// 반드시 소유자 회전으로 언로테이트해서 로컬 스페이스로 내린다.
		const FVector ToAttacker = (Attacker->GetActorLocation() - OwningActor->GetActorLocation()).GetSafeNormal2D();

		if ( ToAttacker.IsNearlyZero() )
			return {};

		return OwningActor->GetActorRotation().UnrotateVector(ToAttacker);
	}
}

FGameplayTagContainer FYSMontageSelector_ByTag::BuildQueryTags(const UYSGameplayAbility* Ability) const
{
	FGameplayTagContainer Query;

	// 공격자가 실어준 히트 강도 등 (Hit.Big / Hit.Normal ...)
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

	// 조회 태그를 가장 구체적으로 만족하는 몽타주를 반환한다.
	const TSoftObjectPtr<UAnimMontage> MontagePtr = TaggedMontage->SelectBest(BuildQueryTags(Ability));

	if ( MontagePtr.IsNull() )
		return nullptr;

	return MontagePtr.LoadSynchronous();
}
