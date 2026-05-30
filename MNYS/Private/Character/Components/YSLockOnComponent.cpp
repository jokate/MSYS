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
	TryReleaseLockOn();
	
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
			double TempDistance = (Player->GetActorLocation() - Actor->GetActorLocation()).SizeSquared();
			
			if ( IsLockOnableTarget(Actor) == false )
				continue;

			// 각도 기준으로 탐색하되, 가장 가까운 대상을 선택한다.
			if ( TempDistance < Distance )
			{
				CurrentLockedTarget = Actor;
				Distance = TempDistance;
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

bool UYSLockOnComponent::IsLockOnableTarget(AActor* Target)
{
	AYSCharacterPlayer* Player = OwnerPlayer.Get();
	
	if ( IsValid(Player) == false )
		return false;
	
	if (Player->GetTeamAttitudeTowards(*Target) != ETeamAttitude::Hostile )
		return false;
			
	IYSBattleActor* BattleActor = Cast<IYSBattleActor>(Target);
	if ( BattleActor == nullptr || BattleActor->IsDead() )
		return false;
			
	FVector DirectionToActor = Target->GetActorLocation() - OwnerPlayer->GetActorLocation();
	
	if ( DirectionToActor.Size() >= MaxLockOnDistance )
		return false;
	
	DirectionToActor.Z = 0.0f; // 수평면에서의 방향만 고려
			
	DirectionToActor.Normalize();

	FVector ForwardVector = OwnerPlayerController->GetControlRotation().Vector();
	ForwardVector.Z = 0.0f;
	ForwardVector.Normalize();

	float AngleToActor = FMath::Acos(FVector::DotProduct(ForwardVector, DirectionToActor));
	AngleToActor = FMath::RadiansToDegrees(AngleToActor);
	
	return AngleToActor <= MaxLockOnAngle;
}

void UYSLockOnComponent::TryReleaseLockOn()
{
	AYSCharacterPlayer* Player = OwnerPlayer.Get();
	
	if ( IsValid(Player) == false )
		return;
	
	UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
	
	if ( IsValid(ASC) == false )
		return;
	
	if ( CurrentLockedTarget.IsValid() )
	{
		if (ASC->HasMatchingGameplayTag(YSTags::BlockLockOn))
		{
			return;
		}
		
		if ( IsLockOnableTarget(CurrentLockedTarget.Get()) == false )
		{
			UE_LOG(LogTemp, Log, TEXT("Release Lock On"))
			CurrentLockedTarget = nullptr;	
		}
	}
}

