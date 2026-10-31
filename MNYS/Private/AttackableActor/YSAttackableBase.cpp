// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSAttackableBase.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "AttackableActor/YSTelegraphActor.h"
#include "Subsystem/YSObjectPoolingSubsystem.h"


AYSAttackableBase::AYSAttackableBase()
{
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

	EffectScale = InHitContext.IsValid() ? InHitContext->GetEffectScale() : 1.f;
	
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
		EffectScale = 1.f;
		DeprocessActivationType();
		GetWorldTimerManager().ClearTimer(DestroyTimerHandle);
		ClearTelegraph();
	}
}

void AYSAttackableBase::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	// 풀이 정리하며 지우거나 풀 밖에서 지워지는 경로.
	_NotifyLinkedActorEnded();
	ClearTelegraph();
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
	ClearTelegraph();
	
	if ( DestroyDelay > 0.f )
	{
		ReturnToPoolAfter(DestroyDelay);
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

	// 한 번 판정은 활성 중에 이미 반환됐을 수 있다. 반환된 액터에 예고를 띄우면 남는다.
	if ( IsPoolActive() )
	{
		ProcessTelegraph();
	}
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
	// 지금은 태그 이벤트가 들어오면 공격이 활성화된다. 이벤트마다 활성화하려면 ActivateTimeBased 같은 형태가 필요할 수 있다.
	OnActivate();
}

void AYSAttackableBase::ProcessTelegraph()
{
	TelegraphActor = SpawnTelegraph(GetWorld(), GetActorTransform(), ActivationType == EYSAttackActivationType::TimeBased ? ActivateTime : 0.f);

	if ( IsValid(TelegraphActor) )
	{
		TelegraphActor->SetOwner(this);
	}
}

void AYSAttackableBase::ClearTelegraph()
{
	if ( IsValid(TelegraphActor) )
	{
		TelegraphActor->Destroy();
	}

	TelegraphActor = nullptr;
}

void AYSAttackableBase::ReturnToPoolAfter(float Delay)
{
	if ( Delay <= 0.f )
	{
		GetWorldTimerManager().ClearTimer(DestroyTimerHandle);
		UYSObjectPoolingSubsystem::ReturnToPool(this);
		return;
	}

	GetWorldTimerManager().SetTimer(DestroyTimerHandle, FTimerDelegate::CreateWeakLambda(this, [this]()
	{
		UYSObjectPoolingSubsystem::ReturnToPool(this);
	}), Delay, false);
}
