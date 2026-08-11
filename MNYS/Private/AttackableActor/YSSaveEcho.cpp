// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSSaveEcho.h"

#include "YSAbilitySystemComponent.h"
#include "Character/Components/YSSaveComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "General/YSGameplayTag.h"

// Sets default values
AYSSaveEcho::AYSSaveEcho()
{
	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	
	AbilitySystemComponent = CreateDefaultSubobject<UYSAbilitySystemComponent>(TEXT("AbilitySystemComponent"));

	// 애초에 이놈 충돌하면 안됨.
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 루트모션은 살린다 — 발도의 돌진을 재현해야 하기 때문이다.
	GetCharacterMovement()->SetMovementMode(MOVE_Walking);
	GetCharacterMovement()->bRunPhysicsWithNoController = true;
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
	
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	AbilitySystemComponent->SetLooseGameplayTagCount(YSTags::State_Echo, 1);

	FGameplayAbilitySpec Spec(InTechnique.AbilityClass, 1);
	ReplayHandle = AbilitySystemComponent->GiveAbility(Spec);

	AbilitySystemComponent->OnAbilityEnded.AddUObject(this, &AYSSaveEcho::OnReplayEnded);

	// 이벤트로 활성하는 이유 — TryActivateAbility 로는 시작 노드를 전달할 방법이 없다.
	// 활성 후에 ActivePlayback 을 따로 부르면 0번 몽타주가 한 프레임 재생됐다 끊긴다.
	FGameplayEventData EventData;
	EventData.EventTag = YSTags::Event_Replay;
	EventData.EventMagnitude = InTechnique.PlaybackIndex;
	EventData.Instigator = InMaster;
	EventData.Target = this;

	AbilitySystemComponent->TriggerAbilityFromGameplayEvent(
		ReplayHandle, AbilitySystemComponent->AbilityActorInfo.Get(),
		YSTags::Event_Replay, &EventData, *AbilitySystemComponent);

	GetWorldTimerManager().SetTimer(LifeTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		Destroy();
	}), MaxLifeTime, false);
}

void AYSSaveEcho::MirrorAppearance(const ACharacter* InMaster)
{
	USkeletalMeshComponent* MasterMesh = InMaster->GetMesh();

	if ( IsValid(MasterMesh) == false )
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
}

void AYSSaveEcho::OnSpawnInitialize(AActor* InOwnerActor, const TSharedPtr<FYSAbilityHitContext>& HitContext)
{
	ACharacter* InMaster = Cast<ACharacter>(InOwnerActor);
	UYSSaveComponent* SaveComponent = UYSSaveComponent::Get(InOwnerActor);

	FYSSavedTechnique Technique;

	if ( IsValid(InMaster) == false || IsValid(SaveComponent) == false || SaveComponent->TryConsumeSlot(Technique) == false )
	{
		Destroy();
		return;
	}

	Initialize(InMaster, Technique);
}

void AYSSaveEcho::OnReplayEnded(const FAbilityEndedData& EndedData)
{
	if ( EndedData.AbilitySpecHandle != ReplayHandle )
	{
		return;
	}

	Destroy();
}

void AYSSaveEcho::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	GetWorldTimerManager().ClearTimer(LifeTimerHandle);

	if ( IsValid(AbilitySystemComponent) )
	{
		AbilitySystemComponent->OnAbilityEnded.RemoveAll(this);
	}

	Super::EndPlay(EndPlayReason);
}
