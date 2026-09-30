// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSAttackableBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "AttackableActor/YSTelegraphActor.h"
#include "Subsystem/YSObjectPoolingSubsystem.h"


// Sets default values
AYSAttackableBase::AYSAttackableBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	
	SceneRoot = CreateDefaultSubobject<USceneComponent>(TEXT("SceneRoot"));
	SetRootComponent(SceneRoot);
	
	RootMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RootMesh"));
	RootMesh->SetupAttachment(SceneRoot);
}

void AYSAttackableBase::AllocateInstigator(AActor* InInstigator)
{
	InstigatorActor = InInstigator;
}

bool AYSAttackableBase::OnSpawnInitialize(AActor* InOwnerActor, AActor* InInstigator, const TSharedPtr<FYSAbilityHitContext>& InHitContext)
{
	OwnerActor = InOwnerActor;
	AllocateInstigator(InInstigator);
	if (InHitContext.IsValid())
	{
		InitializeHitContext(InHitContext);    
	}
	
	return true;
}

void AYSAttackableBase::SetPoolActive(bool bActive)
{
	IYSSpawnInitializable::SetPoolActive(bActive);
	
	if ( bActive )
	{
		ProcessActivationType();
	}
	else
	{
		// 컨텍스트를 놓기 전에 알린다. 안 빼두면 다음 사용이 이 액터를 재사용할 때 이전 컨텍스트의 Release 에 같이 걸린다.
		_NotifyLinkedActorEnded();
		HitContext = nullptr;
		DeprocessActivationType();
		GetWorldTimerManager().ClearTimer(DestroyTimerHandle);
	}
}

void AYSAttackableBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 풀을 거치지 않고 Destroy() 로 끝나는 경로.
	_NotifyLinkedActorEnded();
	Super::EndPlay(EndPlayReason);
}

UAbilitySystemComponent* AYSAttackableBase::_FindEventSourceASC() const
{
	AActor* Current = OwnerActor.Get();

	while ( IsValid(Current) )
	{
		if ( UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Current) )
		{
			return ASC;
		}

		const AYSAttackableBase* Attackable = Cast<AYSAttackableBase>(Current);
		Current = IsValid(Attackable) ? Attackable->OwnerActor.Get() : nullptr;
	}

	return nullptr;
}

void AYSAttackableBase::_NotifyLinkedActorEnded()
{
	if ( HitContext.IsValid() )
	{
		HitContext->NotifyLinkedActorEnded(this);
	}
}

void AYSAttackableBase::OnActivate_Implementation()
{
	if ( IsValid(TelegraphActor) ) 
	{
		TelegraphActor->Destroy();
	}
	
	if ( DestroyDelay > 0.f )
	{
		GetWorldTimerManager().SetTimer(DestroyTimerHandle, this, &AYSAttackableBase::DestroyActor, DestroyDelay, false);	
	}
}

void AYSAttackableBase::ProcessActivationType()
{
	switch (ActivationType)
	{
	case EYSAttackActivationType::Instant :
		{
			OnActivate();
			break;
		}
	case EYSAttackActivationType::TagBased :
		{
			UAbilitySystemComponent* ASC = _FindEventSourceASC();

			if ( IsValid(ASC) )
			{
				TagEventHandle = ASC->GenericGameplayEventCallbacks.FindOrAdd(EventTag).AddUObject(this, &AYSAttackableBase::OnActivateTagCallback);
				// 해제 시점엔 스포너가 이미 사라져 체인을 못 탈 수 있다. 구독한 ASC 를 쥐고 있는다.
				TagEventASC = ASC;
			}
			break;
		}
	case EYSAttackActivationType::TimeBased :
		{
			if ( ActivateTime > 0.f )
			{
				GetWorldTimerManager().SetTimer(ActivateTimerHandle, this, &AYSAttackableBase::OnActivate, ActivateTime, false);	
			}
			break;
		}
	}
	
	ProcessTelegraph();
}

void AYSAttackableBase::DeprocessActivationType()
{
	switch (ActivationType)
	{
	case EYSAttackActivationType::TagBased :
		{
			UAbilitySystemComponent* ASC = TagEventASC.Get();

			// 태그 통째로 지우면 같은 태그를 기다리는 WaitGameplayEvent 태스크까지 날아간다. 내 핸들만 뺀다.
			if ( IsValid(ASC) )
			{
				if ( FGameplayEventMulticastDelegate* Delegate = ASC->GenericGameplayEventCallbacks.Find(EventTag) )
				{
					Delegate->Remove(TagEventHandle);
				}
			}
			TagEventHandle.Reset();
			TagEventASC = nullptr;
		}
	case EYSAttackActivationType::TimeBased :
		{
			GetWorldTimerManager().ClearTimer(ActivateTimerHandle);	
		}
	default : 
		break;
	}
}

void AYSAttackableBase::OnActivateTagCallback(const FGameplayEventData* GameplayEventData)
{	
	//2026.06.15 일단은 태그 이벤트가 들어왔을 때, 공격이 활성화 되는 형태로 만들어 놓긴 했는데,
	//추후에 태그 이벤트가 들어왔을 때마다 공격이 활성화 되는 형태로 만들 수도 있을 것 같긴 함 ( 그럴 경우에는 ActivateTimeBased 같은 형태로 만들어야 할 듯 )
	OnActivate();
}

void AYSAttackableBase::DestroyActor()
{
	UYSObjectPoolingSubsystem* ObjectPoolingSubsystem = UYSObjectPoolingSubsystem::Get(GetWorld());
	if ( IsValid(ObjectPoolingSubsystem) )
	{
		ObjectPoolingSubsystem->ReturnPooledActor(this);
	}
	else
	{
		Destroy();
	}
}
