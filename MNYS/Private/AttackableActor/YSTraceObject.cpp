// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSTraceObject.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "DrawDebugHelpers.h"
#include "GenericTeamAgentInterface.h"
#include "YSBattleActor.h"
#include "YSDeveloperSettings.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "Character/Components/YSCameraManageComponent.h"
#include "General/YSGameplayTag.h"
#include "Library/YSBlueprintFunctionLibrary.h"

UYSTraceObject* UYSTraceObject::Create(UObject* InOuter, AActor* InOwnerActor, AActor* InInstigator, const FYSTraceConfig& InConfig)
{
	UYSTraceObject* TraceObject = NewObject<UYSTraceObject>(InOuter);
	TraceObject->OwnerActor  = InOwnerActor;
	TraceObject->Instigator = InInstigator;
	TraceObject->TraceConfig = InConfig;
	TraceObject->RemainHitProcessCount = InConfig.LimitHitCount;
	return TraceObject;
}

void UYSTraceObject::ExecuteOnce()
{
	Tick(0.f);
}

void UYSTraceObject::Tick(float DeltaTime)
{
	// 트레이싱 비용 자체가 좀 있어서 히트카운트 Deplete 된 경우에는 처리 X
	if ( IsHitCountDepleted() )
		return; 
	
	_TraceByConfig();
}

void UYSTraceObject::ResetHitState()
{
	TracedActors.Reset();
	HitTimeMap.Reset();
	bHasPreviousLocation = false;
	PreviousLocation     = FVector::ZeroVector;
	RemainHitProcessCount = TraceConfig.LimitHitCount;
}

bool UYSTraceObject::DecreaseHitProcessCount()
{
	--RemainHitProcessCount;
	return IsHitCountDepleted();
}

// 히트 카운트 자체가 나간 경우에는 별도 처리 하지 않음.
bool UYSTraceObject::IsHitCountDepleted() const
{
	// 0 인 경우에는 히트에 대한 제한을 따로 두지는 않도록 해야 함.
	if ( TraceConfig.LimitHitCount <= 0)
	{
		return false;
	}
	
	return RemainHitProcessCount <= 0;
}

void UYSTraceObject::_TraceByConfig()
{
	if (!IsValid(OwnerActor))
		return;
	
	UWorld* World = OwnerActor->GetWorld();
	if (!IsValid(World))
		return;

	USkeletalMeshComponent* Mesh = OwnerActor->FindComponentByClass<USkeletalMeshComponent>();

	FVector Location      = OwnerActor->GetActorLocation();
	FTransform Transform  = OwnerActor->GetActorTransform();

	if (TraceConfig.SocketName != NAME_None)
	{
		if (IsValid(Mesh))
		{		
			Transform = Mesh->GetSocketTransform(TraceConfig.SocketName);
			Location  = Transform.GetLocation();
		}
	}
	else if (TraceConfig.RelativeLocation != FVector::ZeroVector)
	{
		Location = Transform.TransformPosition(TraceConfig.RelativeLocation);
	}

	const FVector Start = bHasPreviousLocation ? PreviousLocation : Location;

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(OwnerActor);

	TArray<FHitResult> RawHits;
	bool  bHit    = false;
	FVector LineEnd = Location;

	switch (TraceConfig.Shape)
	{
	case EYSTraceShape::Box:
		bHit = World->SweepMultiByProfile(RawHits, Start, Location, Transform.GetRotation(),
			TraceConfig.CollisionProfile,
			FCollisionShape::MakeBox(TraceConfig.Extent), Params);
		break;

	case EYSTraceShape::Sphere:
		bHit = World->SweepMultiByProfile(RawHits, Start, Location, FQuat::Identity,
			TraceConfig.CollisionProfile,
			FCollisionShape::MakeSphere(TraceConfig.Extent.X), Params);
		break;

	case EYSTraceShape::Capsule:
		bHit = World->SweepMultiByProfile(RawHits, Start, Location, Transform.GetRotation(),
			TraceConfig.CollisionProfile,
			FCollisionShape::MakeCapsule(TraceConfig.Extent.X, TraceConfig.Extent.Y), Params);
		break;

	case EYSTraceShape::Line:
		LineEnd = Location + Transform.GetUnitAxis(EAxis::X) * TraceConfig.Extent.X;
		bHit    = World->LineTraceMultiByProfile(RawHits, Location, LineEnd,
			TraceConfig.CollisionProfile, Params);
		break;
	}

	FVector Direction = (Start - Location).GetSafeNormal();
	EffectRotation = Direction.ToOrientationRotator();
	
	PreviousLocation     = Location;
	bHasPreviousLocation = true;
	
	const float  CurrentTime = World->GetTimeSeconds();
	TArray<FHitResult> ValidHits;
	
	if (bHit)
	{
		for (const FHitResult& Hit : RawHits)
		{
			AActor* HitActor = Hit.GetActor();
			if (!IsValid(HitActor)) continue;
			
			if (TraceConfig.bContinuousHit)
			{
				if (const float* LastHitTime = HitTimeMap.Find(HitActor))
				{
					if (CurrentTime - *LastHitTime < TraceConfig.HitTimeCheck) continue;
				}
				
				HitTimeMap.Add(HitActor, CurrentTime);
			}
			else
			{
				if (TracedActors.Contains(HitActor)) continue;
				TracedActors.Add(HitActor);
			}

			ValidHits.Add(Hit);
		}
	}

#if ENABLE_DRAW_DEBUG
	if (TraceConfig.bDebugDraw)
	{
		const FColor ShapeColor = ValidHits.Num() > 0 ? FColor::Red
		                        : bHit               ? FColor::Orange
		                                             : FColor::Green;
		const float Duration  = TraceConfig.DebugDrawDuration;
		const FQuat Rotation  = Transform.GetRotation();

		switch (TraceConfig.Shape)
		{
		case EYSTraceShape::Box:
			DrawDebugBox(World, Start,    TraceConfig.Extent, Rotation, ShapeColor, false, Duration);
			if (!Start.Equals(Location))
				DrawDebugBox(World, Location, TraceConfig.Extent, Rotation, ShapeColor, false, Duration);
			DrawDebugLine(World, Start, Location, ShapeColor, false, Duration, 0, 1.f);
			break;

		case EYSTraceShape::Sphere:
			DrawDebugSphere(World, Start,    TraceConfig.Extent.X, 12, ShapeColor, false, Duration);
			if (!Start.Equals(Location))
				DrawDebugSphere(World, Location, TraceConfig.Extent.X, 12, ShapeColor, false, Duration);
			DrawDebugLine(World, Start, Location, ShapeColor, false, Duration, 0, 1.f);
			break;

		case EYSTraceShape::Capsule:
			DrawDebugCapsule(World, Start,    TraceConfig.Extent.Y, TraceConfig.Extent.X, Rotation, ShapeColor, false, Duration);
			if (!Start.Equals(Location))
				DrawDebugCapsule(World, Location, TraceConfig.Extent.Y, TraceConfig.Extent.X, Rotation, ShapeColor, false, Duration);
			DrawDebugLine(World, Start, Location, ShapeColor, false, Duration, 0, 1.f);
			break;

		case EYSTraceShape::Line:
			DrawDebugLine(World, Location, LineEnd, ShapeColor, false, Duration, 0, 2.f);
			break;
		}

		for (const FHitResult& Hit : ValidHits)
		{
			DrawDebugSphere(World, Hit.ImpactPoint, 5.f, 8, FColor::Yellow, false, FMath::Max(Duration, 0.1f));
		}
	}
#endif

	if (ValidHits.Num() > 0)
	{
		_ProcessValidHit(ValidHits);
	}
}

void UYSTraceObject::_ProcessValidHit(const TArray<FHitResult>& InProcessedHits)
{
	if (!IsValid(Instigator))
		return;

	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);
	if (!IsValid(OwnerASC))
		return;

	const UYSCharacterAttributeSetBase* OwnerAttribute = OwnerASC->GetSet<UYSCharacterAttributeSetBase>();
	if (!IsValid(OwnerAttribute))
		return;

	IGenericTeamAgentInterface* TeamAgent = Cast<IGenericTeamAgentInterface>(Instigator);
	if (TeamAgent == nullptr)
		return;
	
	const UYSDeveloperSettings* DevSetting = GetDefault<UYSDeveloperSettings>();
	
	if ( IsValid(DevSetting) == false )
		return;
	
	TArray<FHitResult> ValidHits;

	for (const FHitResult& HitResult : InProcessedHits)
	{
		AActor* HitActor = HitResult.GetActor();
		if (!IsValid(HitActor))
			continue;

		if (TeamAgent->GetTeamAttitudeTowards(*HitActor) == ETeamAttitude::Friendly)
			continue;

		IYSBattleActor* BattleActor = Cast<IYSBattleActor>(HitActor);
		if (BattleActor == nullptr || BattleActor->IsDead())
			continue;

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if (!IsValid(TargetASC))
			continue;

		const UYSCharacterAttributeSetBase* TargetAttribute = TargetASC->GetSet<UYSCharacterAttributeSetBase>();
		if (!IsValid(TargetAttribute))
			continue;

		// JustAvoid 윈도우 처리 -> 여기서는 그냥 Effect를 부여하는게 좋을까?
		// 버프를 부여한다. 이 시점에 버프의 경우에는 이펙트 출력을 담당하기로 합시당
		if (TargetASC->HasMatchingGameplayTag(YSTags::JustAvoid_Window))
		{
			FGameplayEffectContextHandle EffectContextHandle = OwnerASC->MakeEffectContext();
			EffectContextHandle.AddInstigator(Instigator, Instigator);
			FGameplayEffectSpec Spec(DevSetting->JustAvoidGameplayEffect.GetDefaultObject(), EffectContextHandle);	
			Spec.AddDynamicAssetTag(YSTags::Event_JustAvoid);
			
			OwnerASC->ApplyGameplayEffectSpecToTarget(Spec, TargetASC);
			continue;
		}

		// 무적 상태 스킵
		if (TargetASC->HasMatchingGameplayTag(YSTags::Invincible))
			continue;

		// 데미지 이벤트 전송
		UYSBlueprintFunctionLibrary::SendHitEventToTarget(Instigator, HitActor, TraceConfig.DamageRow);
		UYSBlueprintFunctionLibrary::SpawnEffects(Instigator, TraceConfig.DamageRow, HitResult.ImpactPoint, EffectRotation);
		UYSBlueprintFunctionLibrary::ApplyHitEffects(OwnerActor, Instigator, HitActor, TraceConfig.DamageRow, HitResult);
		
		ValidHits.Add(HitResult);
		
		if ( DecreaseHitProcessCount() )
		{
			OnHitCountDepleted.Broadcast();
			break;
		}
	}
	
	if ( ValidHits.Num() > 0 )
	{
		OnTraceHit.Broadcast(ValidHits, TraceConfig.DamageRow);
	}
}
