// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/GameplayEffectComponents/YSApplyNiagaraEffectGEComponent.h"

#include "AbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "NiagaraComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "GameFramework/Character.h"

bool UYSApplyNiagaraEffectGEComponent::OnActiveGameplayEffectAdded(FActiveGameplayEffectsContainer& ActiveGEContainer,
	FActiveGameplayEffect& ActiveGE) const
{
	const bool bActive = Super::OnActiveGameplayEffectAdded(ActiveGEContainer, ActiveGE);

	UAbilitySystemComponent* ASC = ActiveGEContainer.Owner;

	if ( IsValid(ASC) == false || IsValid(TargetToSpawn) == false )
		return bActive;

	ACharacter* Character = Cast<ACharacter>(ASC->GetOwnerActor());

	if ( IsValid(Character) == false)
	{
		return bActive;
	}

	UNiagaraComponent* SpawnedComponent = UNiagaraFunctionLibrary::SpawnSystemAttached(TargetToSpawn, Character->GetMesh(), Socket,
		FVector::ZeroVector, FRotator::ZeroRotator, EAttachLocation::SnapToTarget, true);

	if ( IsValid(SpawnedComponent) == false )
	{
		return bActive;
	}

	// 이 컴포넌트는 여러 캐릭터가 같이 쓰는 GE 애셋에 있어 상태를 둘 수 없으므로, 이펙트를 이 활성 GE 의 제거에 묶는다.
	ActiveGE.EventSet.OnEffectRemoved.AddWeakLambda(SpawnedComponent, [SpawnedComponent](const FGameplayEffectRemovalInfo&)
	{
		SpawnedComponent->DestroyInstance();
	});

	return bActive;
}
