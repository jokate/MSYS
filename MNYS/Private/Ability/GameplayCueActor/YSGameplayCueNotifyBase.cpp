// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayCueActor/YSGameplayCueNotifyBase.h"

#include "Ability/GameplayCueActor/GameplayCueEvent/YSGameplayCueActionBase.h"


// Sets default values
AYSGameplayCueNotifyBase::AYSGameplayCueNotifyBase()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

bool AYSGameplayCueNotifyBase::OnActive_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnActive_Implementation(MyTarget, Parameters);
	
	for ( UYSGameplayCueActionBase* Action : GameplayCueNotifyList )
	{
		if ( IsValid(Action) == false )
			continue;
		
		Action->OnActive(this, MyTarget, Parameters);
	}
	
	return true;
}

bool AYSGameplayCueNotifyBase::OnRemove_Implementation(AActor* MyTarget, const FGameplayCueParameters& Parameters)
{
	Super::OnRemove_Implementation(MyTarget, Parameters);
	
	for ( UYSGameplayCueActionBase* Action : GameplayCueNotifyList )
	{
		if ( IsValid(Action) == false )
			continue;
		
		Action->OnRemove(this, MyTarget, Parameters);
	}
	
	return true;
}

