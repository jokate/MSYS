// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSAttackableActor.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "YSBattleActor.h"
#include "AttackableActor/YSTraceObject.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "General/YSGameplayTag.h"
#include "Library/YSBlueprintFunctionLibrary.h"


// Sets default values
AYSAttackableActor::AYSAttackableActor()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	
	RootMesh->SetupAttachment(SceneRoot);
}

void AYSAttackableActor::AllocateInstigator(AActor* InInstigator)
{
	OwnerActor = InInstigator;
	TraceObject = UYSTraceObject::Create(this, this, InInstigator, TraceConfig);
	TraceObject->OnTraceHit.AddDynamic(this, &AYSAttackableActor::_OnTraceObjectHit);
	TraceObject->OnHitCountDepleted.AddDynamic(this, &AYSAttackableActor::_OnHitCountDepleted);
}

// Called when the game starts or when spawned
void AYSAttackableActor::BeginPlay()
{
	Super::BeginPlay();
	
	if ( IsValid(TraceObject) )
	{
		if (TraceConfig.bTraceOnce)
		{
			TraceObject->ExecuteOnce();
			Destroy();
		}	
	}
}

void AYSAttackableActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (IsValid(TraceObject))
	{
		TraceObject->OnTraceHit.RemoveAll(this);
		TraceObject = nullptr;
	}
	
	Super::EndPlay(EndPlayReason);
}

void AYSAttackableActor::_OnHitCountDepleted()
{
	Destroy();
}

// Called every frame
void AYSAttackableActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	
	_Trace(DeltaTime);
}

void AYSAttackableActor::_Trace(float DeltaTime)
{
	if (IsValid(TraceObject))
	{
		TraceObject->Tick(DeltaTime);
	}
}

void AYSAttackableActor::_OnTraceObjectHit(const TArray<FHitResult>& HitResults, const FName& DamageRow)
{
	// 데미지 처리 로직을 여기에서 수행해야 한다.
	AActor* InstigatorPtr = OwnerActor.Get();
	
	if (IsValid(InstigatorPtr) == false )
	{
		return;
	}
	
	UAbilitySystemComponent* OwnerASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(InstigatorPtr);
	if ( IsValid(OwnerASC) == false )
		return;

	const UYSCharacterAttributeSetBase* OwnerAttribute = OwnerASC->GetSet<UYSCharacterAttributeSetBase>();
	if ( IsValid(OwnerAttribute) == false )
		return;

	IGenericTeamAgentInterface* TeamAgentInterface = Cast<IGenericTeamAgentInterface>(InstigatorPtr);
	if ( TeamAgentInterface == nullptr )
		return;
	
	for ( const FHitResult& HitResult : HitResults ) 
	{
		AActor* HitActor = HitResult.GetActor();

		if (TeamAgentInterface->GetTeamAttitudeTowards(*HitActor) == ETeamAttitude::Friendly )
			continue;
		
		if ( IsValid(HitActor) == false ) 
			continue;
		
		IYSBattleActor* BattleActor = Cast<IYSBattleActor>(HitActor);
		if (BattleActor == nullptr || BattleActor->IsDead())
		{
			continue;
		}
		
		UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
		if ( IsValid(ASC) == false )
			continue;
		
		const UYSCharacterAttributeSetBase* TargetAttributeSet = ASC->GetSet<UYSCharacterAttributeSetBase>();
		if ( IsValid(TargetAttributeSet) == false )
			continue;
		
		// 만약 회피 윈도우에 걸린 경우. (이거 고민좀 해봐야 할 거 같음 )
		if (ASC->HasMatchingGameplayTag(YSTags::JustAvoid_Window))
		{
			FGameplayEventData GameplayEventData;
			GameplayEventData.Instigator = InstigatorPtr;
			GameplayEventData.ContextHandle.AddHitResult(HitResult);
			
			ASC->HandleGameplayEvent(YSTags::Event_JustAvoid, &GameplayEventData);
			continue;
		}
		
		if (ASC->HasMatchingGameplayTag(YSTags::Invincible))
		{
			continue;
		}
		
		// 데미지 히트에 따른 이벤트 송신.
		UYSBlueprintFunctionLibrary::SendHitEventToTarget(InstigatorPtr, HitActor, DamageRow);
		
		if ( IsValid(TraceObject) )
		{
			TraceObject->DecreaseHitProcessCount();
		}
	}
}

