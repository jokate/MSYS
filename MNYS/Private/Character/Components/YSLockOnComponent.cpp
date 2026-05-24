// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/YSLockOnComponent.h"

#include "YSAbilitySystemComponent.h"
#include "Character/YSCharacterPlayer.h"
#include "Character/YSPlayerController.h"
#include "General/YSGameplayTag.h"
#include "Kismet/KismetSystemLibrary.h"


// Sets default values for this component's properties
UYSLockOnComponent::UYSLockOnComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}

UYSLockOnComponent* UYSLockOnComponent::Get(const AActor* Character)
{	
	if ( IsValid(Character) == false )
		return nullptr;
	
	AYSPlayerController* PC = Character->GetInstigatorController<AYSPlayerController>();
	
	if ( IsValid(PC) == false ) 
		return nullptr;
	
	return PC->LockOnComponent;
}


// Called when the game starts
void UYSLockOnComponent::BeginPlay()
{
	Super::BeginPlay();

	AYSPlayerController* PlayerController = Cast<AYSPlayerController>(GetOwner());
	
	if ( IsValid(PlayerController) )
	{
		OwnerPlayer = Cast<AYSCharacterPlayer>(PlayerController->GetPawn());
		OwnerPlayerController = PlayerController;
	}
}


// Called every frame
void UYSLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                       FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	ProcessLockOnFunction(DeltaTime);
}

void UYSLockOnComponent::ForceSetLockOn(AActor* TargetToLockOn)
{
	CurrentLockedTarget = TargetToLockOn;
}

void UYSLockOnComponent::ProcessLockOnFunction(float DeltaTime)
{
	if ( OwnerPlayer.IsValid() == false )
		return;
	
	// 락온 잡혀있으면 락온 X
	if ( CurrentLockedTarget.IsValid() )
	{
		IYSBattleActor* BattleActor = Cast<IYSBattleActor>(CurrentLockedTarget.Get());
		if ( BattleActor == nullptr || BattleActor->IsDead() )
		{
			CurrentLockedTarget = nullptr;
		}
	}
	
	if ( CurrentLockedTarget.IsValid() )
	{
		ChaseCamera(DeltaTime);
		return;
	}
	
	FindTarget();
}

void UYSLockOnComponent::FindTarget()
{
	if ( OwnerPlayer.IsValid() == false )
		return;
	
	
	AYSCharacterPlayer* Player = OwnerPlayer.Get();
	TArray<AActor*> OverlappedActors;
	if ( UKismetSystemLibrary::SphereOverlapActors(GetWorld(), OwnerPlayer->GetActorLocation(), MaxLockOnDistance, 
		{}, nullptr, {GetOwner()}, OverlappedActors))
	{
		double Distance = MaxLockOnDistance * MaxLockOnDistance;
		for ( AActor* Actor : OverlappedActors )
		{
			if (Player->GetTeamAttitudeTowards(*Actor) != ETeamAttitude::Hostile )
				continue;
			
			UYSAbilitySystemComponent* ASC = UYSAbilitySystemComponent::Get(Actor);
			
			if ( IsValid(ASC) == false )
				continue;
			
			IYSBattleActor* BattleActor = Cast<IYSBattleActor>(Actor);
			if ( BattleActor == nullptr || BattleActor->IsDead() )
				continue;
			
			FVector DirectionToActor = Actor->GetActorLocation() - OwnerPlayer->GetActorLocation();
			double TempDist = DirectionToActor.SizeSquared();
			DirectionToActor.Z = 0.0f; // 수평면에서의 방향만 고려
			
			DirectionToActor.Normalize();

			FVector ForwardVector = OwnerPlayer->GetActorForwardVector();
			ForwardVector.Z = 0.0f;
			ForwardVector.Normalize();

			float AngleToActor = FMath::Acos(FVector::DotProduct(ForwardVector, DirectionToActor));
			AngleToActor = FMath::RadiansToDegrees(AngleToActor);

			// 각도 기준으로 탐색하되, 가장 가까운 대상을 선택한다.
			if ( AngleToActor <= MaxLockOnAngle && TempDist < Distance )
			{
				CurrentLockedTarget = Actor;
				Distance = TempDist;
			}
		}
	}
}

void UYSLockOnComponent::ChaseCamera(float DeltaTime)
{
	if ( OwnerPlayerController.IsValid() == false || CurrentLockedTarget.IsValid() == false )
		return;
	
	FVector ToTarget = (CurrentLockedTarget->GetActorLocation() - OwnerPlayer->GetActorLocation()).GetSafeNormal();
	FRotator TargetRot  = ToTarget.Rotation();
	FRotator CurrentRot = OwnerPlayerController->GetControlRotation();
	
	TargetRot.Pitch = CurrentRot.Pitch;
	OwnerPlayerController->SetControlRotation(
		FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, CameraInterpSpeed));
}

