// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSSaveEcho.h"

#include "YSAbilitySystemComponent.h"
#include "Character/Components/YSSaveComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "General/YSGameplayTag.h"
#include "Subsystem/YSObjectPoolingSubsystem.h"

// Sets default values
AYSSaveEcho::AYSSaveEcho()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	AbilitySystemComponent = CreateDefaultSubobject<UYSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// NoCollision 이면 PhysWalking 이 쿼리 콜리전 게이트에서 즉시 리턴해 루트모션이 위치를 못 옮긴다.
	// 바닥 스윕만 살리고 나머지는 EchoBody 프로파일에서 전부 무시한다.
	GetCapsuleComponent()->SetCollisionProfileName(TEXT("EchoBody"));
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 루트모션은 살린다 — 발도의 돌진을 재현해야 하기 때문이다.
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->bRunPhysicsWithNoController = true;
	GetCharacterMovement()->GravityScale = 0.f;
}

UAbilitySystemComponent* AYSSaveEcho::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

AActor* AYSSaveEcho::GetDamageInstigator() const
{
	return Master.Get();
}
// 디플로이 시킨다.
void AYSSaveEcho::Initialize(ACharacter* InMaster, const FYSSavedTechnique& InTechnique)
{
	if ( IsValid(InMaster) == false || InTechnique.IsValid() == false )
	{
		Destroy();
		return;
	}

	Master = InMaster;
	MirrorAppearance(InMaster);
	
	if ( AbilitySystemComponent->AbilityActorInfo.IsValid() == false )
	{
		// 이미 BeginPlay 가 끝난 상태라면, AbilitySystemComponent 초기화가 끝났을 것이다.
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
	
	AbilitySystemComponent->SetLooseGameplayTagCount(YSTags::State_Echo, 1);

	FGameplayAbilitySpec Spec(InTechnique.AbilityClass, 1);
	ReplayHandle = AbilitySystemComponent->GiveAbility(Spec);

	AbilitySystemComponent->OnAbilityEnded.AddUObject(this, &AYSSaveEcho::OnReplayEnded);

	GetWorldTimerManager().SetTimer(LifeTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		ReturnToPoolActor();
	}), MaxLifeTime, false);
	
	SavedTechnique = InTechnique;
}


void AYSSaveEcho::MirrorAppearance(const ACharacter* InMaster)
{
	USkeletalMeshComponent* MasterMesh = InMaster->GetMesh();
	UCapsuleComponent* MasterCapsule = InMaster->GetCapsuleComponent();
	if ( IsValid(MasterMesh) == false  || IsValid(MasterCapsule) == false )
	{
		return;
	}

	USkeletalMeshComponent* EchoMesh = GetMesh();
	EchoMesh->SetSkeletalMesh(MasterMesh->GetSkeletalMeshAsset());
	EchoMesh->SetAnimInstanceClass(MasterMesh->GetAnimClass());
	EchoMesh->SetRelativeTransform(MasterMesh->GetRelativeTransform());
	
	if ( IsValid(EchoOverlayMaterial) )
	{
		EchoMesh->SetOverlayMaterial(EchoOverlayMaterial);
	}
	
	UCapsuleComponent* EchoCapsule = GetCapsuleComponent();
	EchoCapsule->SetCapsuleHalfHeight(MasterCapsule->GetUnscaledCapsuleHalfHeight());
	EchoCapsule->SetCapsuleRadius(MasterCapsule->GetUnscaledCapsuleRadius());
	
	FVector ActorLocation = GetActorLocation();
	ActorLocation.Z += EchoCapsule->GetUnscaledCapsuleHalfHeight();
	SetActorLocation(ActorLocation);
}

bool AYSSaveEcho::OnSpawnInitialize(AActor* InOwnerActor, AActor* InInstigator, const TSharedPtr<FYSAbilityHitContext>& HitContext)
{
	ACharacter* InMaster = Cast<ACharacter>(InOwnerActor);
	UYSSaveComponent* SaveComponent = UYSSaveComponent::Get(InOwnerActor);

	FYSSavedTechnique Technique;

	if ( IsValid(InMaster) == false || IsValid(SaveComponent) == false || SaveComponent->TryConsumeSlot(Technique) == false )
	{
		return false;
	}

	Initialize(InMaster, Technique);
	return true;
}

void AYSSaveEcho::SetPoolActive(bool bActive)
{
	IYSSpawnInitializable::SetPoolActive(bActive);

	if ( bActive )
	{
		// 이벤트로 활성하는 이유 — TryActivateAbility 로는 시작 노드를 전달할 방법이 없다.
		// 활성 후에 ActivePlayback 을 따로 부르면 0번 몽타주가 한 프레임 재생됐다 끊긴다.
		FGameplayEventData EventData;
		EventData.EventTag = YSTags::Event_Replay;
		EventData.EventMagnitude = SavedTechnique.PlaybackIndex;
		EventData.Instigator = Master.Get();
		EventData.Target = this;

		AbilitySystemComponent->TriggerAbilityFromGameplayEvent(
			ReplayHandle, AbilitySystemComponent->AbilityActorInfo.Get(),
			YSTags::Event_Replay, &EventData, *AbilitySystemComponent);
		
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	}
	else
	{
		AbilitySystemComponent->RemoveLooseGameplayTag(YSTags::State_Echo);
		AbilitySystemComponent->OnAbilityEnded.RemoveAll(this);	
		AbilitySystemComponent->ClearAbility(ReplayHandle);	
		GetWorldTimerManager().ClearTimer(LifeTimerHandle);
		
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->StopMovementImmediately();
	}
}

void AYSSaveEcho::ReturnToPoolActor()
{
	UYSObjectPoolingSubsystem* PoolingSubsystem = GetWorld()->GetSubsystem<UYSObjectPoolingSubsystem>();
	if ( IsValid(PoolingSubsystem) )
	{
		PoolingSubsystem->ReturnPooledActor(this);
	}
	else
	{
		Destroy();
	}
}

void AYSSaveEcho::OnReplayEnded(const FAbilityEndedData& EndedData)
{
	if ( EndedData.AbilitySpecHandle != ReplayHandle )
	{
		return;
	}

	ReturnToPoolActor();
}
