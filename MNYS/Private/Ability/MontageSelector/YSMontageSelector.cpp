// Fill out your copyright notice in the Description page of Project Settings.

#include "Ability/MontageSelector/YSMontageSelector.h"
#include "Ability/YSGameplayAbility.h"
#include "Character/Components/YSCharacterMovementComponent.h"
#include "MotionWarp/UYSMotionWarpingComponent.h"

void FYSMontageSelector::SetMotionWarp(const UYSGameplayAbility* Ability, bool bInSet)
{
	if ( !IsValid(Ability) == false )
		return;

	AActor* AvatarActor = Ability->GetAvatarActorFromActorInfo();

	if ( IsValid(AvatarActor) == false )
		return;

	UYSMotionWarpingComponent* MotionWarpingComponent = UYSMotionWarpingComponent::GetMotionWarpingComponent(AvatarActor);

	if ( IsValid(MotionWarpingComponent) == false )
		return;

	bInSet ? MotionWarpingComponent->SetMotionWarp(MotionWarpName, MotionWarpType) : MotionWarpingComponent->ReleaseMotionWarp(MotionWarpName);
}

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

	if (!IsValid(Ability))
		return GetMontage(EYSMoveDirection::Forward);

	AActor* AvatarActor = Ability->GetAvatarActorFromActorInfo();
	UYSCharacterMovementComponent* Movement = UYSCharacterMovementComponent::Get(AvatarActor);

	if (!IsValid(Movement))
		return GetMontage(EYSMoveDirection::Forward);

	FVector LocalVelocity = Movement->GetLocalSpaceVelocity();
	LocalVelocity.Z = 0.f;

	if (LocalVelocity.IsNearlyZero())
		return GetMontage(EYSMoveDirection::Forward);

	// atan2(Y, X): Y = 로컬 우측, X = 로컬 전방  →  [-180, 180]
	// +22.5° 오프셋으로 섹터 경계를 각 방향 중심에 정렬한 뒤 [0, 360) 으로 정규화
	const float Angle           = FMath::RadiansToDegrees(FMath::Atan2(LocalVelocity.Y, LocalVelocity.X));
	const float NormalizedAngle = FMath::Fmod(Angle + 22.5f + 360.f, 360.f);
	const int32 SectorIndex     = FMath::FloorToInt(NormalizedAngle / 45.f) % 8;

	return GetMontage(static_cast<EYSMoveDirection>(SectorIndex));
}
