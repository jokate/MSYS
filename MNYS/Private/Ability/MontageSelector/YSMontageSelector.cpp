// Fill out your copyright notice in the Description page of Project Settings.

#include "Ability/MontageSelector/YSMontageSelector.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/MotionWarp/YSMotionWarpParam.h"
#include "Character/YSCharacterBase.h"
#include "Character/Components/YSCharacterMovementComponent.h"
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

UAnimMontage* FYSMontageSelector_ByTag::SelectMontage(const UYSGameplayAbility* Ability) const
{
	if (!IsValid(Ability))
		return nullptr;

	// 태그 매칭이 가장 높은 몽타주를 반환합니다.
	const FGameplayEventData* EventData = Ability->GetEventData();
	
	if (EventData == nullptr)
		return nullptr;
	
	for ( const FGameplayTag& Tag : EventData->TargetTags )
	{
		if ( TagMontages.Contains(Tag) == false )
			return nullptr;	
		
		return TagMontages[Tag].IsNull() ? nullptr : TagMontages[Tag].LoadSynchronous();
	}
	
	return nullptr;
}
