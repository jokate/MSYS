// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/YSEnhancedInputComponent.h"


// Sets default values for this component's properties
UYSEnhancedInputComponent::UYSEnhancedInputComponent()
{
}

void UYSEnhancedInputComponent::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void UYSEnhancedInputComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                              FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

}

