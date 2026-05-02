// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/TargetActor/YSTA_Trace.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "Components/ShapeComponent.h"
#include "GameFramework/Character.h"


class IGenericTeamAgentInterface;
// Sets default values
AYSTA_Trace::AYSTA_Trace()
{
	RootCollisionComponent = CreateDefaultSubobject<USceneComponent>("RootCollisionComponent");
	SetRootComponent(RootCollisionComponent);
	PrimaryActorTick.bCanEverTick = true;
}

void AYSTA_Trace::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	TArray<USceneComponent*> ChildrenComponents;
	RootCollisionComponent->GetChildrenComponents(true, ChildrenComponents);

	for ( USceneComponent* Child : ChildrenComponents )
	{
		UShapeComponent* ShapeComponent = Cast<UShapeComponent>(Child);

		if ( IsValid(ShapeComponent) )
		{
			ShapeComponent->SetCollisionProfileName(TEXT("Weapon"));
			ShapeComponent->SetCollisionEnabled(ECollisionEnabled::Type::QueryAndPhysics);
			DamageShapes.Add(ShapeComponent);
		}
	}
}

void AYSTA_Trace::BeginPlay()
{
	Super::BeginPlay();
}

void AYSTA_Trace::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);
	
	if ( IsValid(SourceActor) )
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
	}
	
	Super::EndPlay(EndPlayReason);
}

void AYSTA_Trace::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	TraceStart();
}

void AYSTA_Trace::StartTargeting(UGameplayAbility* Ability)
{
	Super::StartTargeting(Ability);
	SourceActor = Ability->GetCurrentActorInfo()->AvatarActor.Get();

	UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(SourceActor);
	/*FMUDamageInfo DamageInfo;
	
	if (!ASC || !UMUFunctionLibrary::GetDamageInfo(this, TargetDamageInfo, DamageInfo))
	{
		return;	
	}*/
	
	//ASC->GenericGameplayEventCallbacks.FindOrAdd(MU_EVENT_TRACEEND).AddUObject(this, &ThisClass::OnAnimNotifyStateEnd);
}

FGameplayAbilityTargetDataHandle AYSTA_Trace::MakeTargetData()
{
	return FGameplayAbilityTargetDataHandle();
}

void AYSTA_Trace::ConfirmTargetingAndContinue()
{
	if (IsValid(SourceActor))
	{
		FGameplayAbilityTargetDataHandle DataHandle = MakeTargetData();
		
		if (TargetDataReadyDelegate.IsBound())
		{
			TargetDataReadyDelegate.Broadcast(DataHandle);
		}
	}
}

void AYSTA_Trace::OnAnimNotifyStateEnd(const FGameplayEventData* EventData)
{
	ConfirmTargeting();
}

void AYSTA_Trace::TraceStart()
{
	//Redefine : 2026-03-01 : Check All Shape Overlap Actors
	TArray<FHitResult> HitResults;
	for ( UShapeComponent* DamageShape : DamageShapes )
	{
		TArray<FOverlapInfo> OverlapInfos = DamageShape->GetOverlapInfos();

		for ( FOverlapInfo& OverlapInfo : OverlapInfos )
		{
			FHitResult HitResult = OverlapInfo.OverlapInfo;

			if (SourceActor == HitResult.GetActor())
			{
				continue;
			}

			HitResults.Add(HitResult);
		}
	}

	ProcessHitResult(HitResults);
}

void AYSTA_Trace::InitializeData(const FName& InTargetDamageInfo)
{
	TargetDamageInfo = InTargetDamageInfo;
}

void AYSTA_Trace::ProcessHitResult(const TArray<FHitResult>& HitResults)
{
	ACharacter* Character = CastChecked<ACharacter>(SourceActor);
	
	const IGenericTeamAgentInterface* SourceActorTeam = CastChecked<IGenericTeamAgentInterface>(SourceActor);

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Character);
	
	for (const auto& HitResult : HitResults)
	{
		if (AActor* HitActor = HitResult.GetActor())
		{
			if (SourceActorTeam->GetTeamAttitudeTowards(*HitActor) != ETeamAttitude::Hostile)
			{
				continue;
			}
			
			if (!QueryActors.Contains(HitActor))
			{
				QueryActors.Add(HitActor);

				// 쿼리 액터에 없는 경우 (판단이 아직 안된 객체의 경우 최초 감지 시, HitResult와 함께 넘겨준다.)
				
				FGameplayEventData GameplayEventData;
				GameplayEventData.Instigator = OwningAbility->GetAvatarActorFromActorInfo();
				FGameplayAbilityTargetData_SingleTargetHit* SingleTargetHit = new FGameplayAbilityTargetData_SingleTargetHit(HitResult);
				
				GameplayEventData.TargetData.Add(SingleTargetHit);

				//Hit가 된 캐릭터에게 즉각적으로 데미지를 가하는 로직.
				UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(HitActor);
				if ( IsValid(SourceASC) == true ) 
				{
					ProcessDamage(SourceASC, TargetASC);
				}
				
				//UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(HitActor, MU_EVENT_ONHIT, GameplayEventData);
			}
		}
	} 
}

void AYSTA_Trace::ProcessDamage(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC)
{
}

void AYSTA_Trace::ApplyBuff(UAbilitySystemComponent* SourceASC, UAbilitySystemComponent* TargetASC, TSubclassOf<UGameplayEffect> BuffEffectClass)
{
	if ( IsValid(SourceASC) == false || IsValid(TargetASC) == false || BuffEffectClass == nullptr ) 
	{
		return;
	}
	
	FGameplayEffectContextHandle SourceEffectContext = SourceASC->MakeEffectContext();
	FGameplayEffectSpecHandle SourceEffectSpecHandle = SourceASC->MakeOutgoingSpec(BuffEffectClass, 1, SourceEffectContext);

	if (SourceEffectSpecHandle.IsValid())
	{
		SourceASC->ApplyGameplayEffectSpecToTarget(*SourceEffectSpecHandle.Data.Get(), TargetASC);
	}
}


