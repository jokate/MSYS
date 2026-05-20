// Fill out your copyright notice in the Description page of Project Settings.


#include "AnimNotify/YSAnimNotify_TransitionState.h"

#include "Input/StateMachine/YSInputStateMachineComponent.h"

void UYSAnimNotify_TransitionState::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation,
                                           const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);
	
	AActor* OwnerActor = MeshComp->GetOwner();
	
	if ( IsValid(OwnerActor) )
	{
		UYSInputStateMachineComponent* StateMachineComponent = UYSInputStateMachineComponent::Get(OwnerActor);
		
		if ( IsValid(StateMachineComponent) )
			StateMachineComponent->TransitionState(NextState);
	}
}
