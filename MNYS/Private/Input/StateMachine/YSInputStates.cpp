// Fill out your copyright notice in the Description page of Project Settings.


#include "Input/StateMachine/YSInputStates.h"

#include "AbilitySystemInterface.h"
#include "YSAbilitySystemComponent.h"
#include "GameplayEffect.h"
#include "Ability/GameplayEffectComponents/YSInputStateGEComponent.h"
#include "General/YSGeneratedGameplayTags.h"
#include "General/YSInputGameplayTags.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"

void UYSInputStates::ProcessInput(const FGameplayTag& InputGameplayTag, EYSInputPhase InputPhase)
{
	UYSAbilitySystemComponent* ASC = OwnerASC.Get();
	UYSInputStateMachineComponent* InputStateMachineComponent = OwnerStateMachineManager.Get();
	if ( IsValid(ASC) == false || IsValid(InputStateMachineComponent) == false )
	{
		return;
	}

	const FYSInputWindowRule* WindowRule = GetWindowRule();

	// 조준 중엔 창이 아니라 조준 상태가 해석한다 (창 안에서도 LMB 는 사격).
	if ( WindowRule != nullptr
		&& WindowRule->AimHeldPassThroughInputs.Contains(InputGameplayTag)
		&& InputStateMachineComponent->IsInputHeld(YSInputTags::InputAim) )
	{
		UYSInputStates* AimState = InputStateMachineComponent->FindState(InputStateMachineComponent->AimInputState);

		if ( IsValid(AimState) && AimState != this )
		{
			AimState->ProcessInput(InputGameplayTag, InputPhase);
			return;
		}
	}

	TArray<FGameplayTag> Candidates;
	ResolveStateTags(InputGameplayTag, Candidates);

	if ( Candidates.Num() == 0 )
	{
		return;
	}

	// 히스토리는 세부 태그로만 남긴다. FYSPlaybackCondition_Input 이 정확 비교라 후보를 다 넣으면 어느 것을 조건에 써야 할지 모호하다.
	InputStateMachineComponent->RecordInputHistory(Candidates[0], InputPhase);

	const bool bHandled = ASC->ProcessAbilityByInputPass(Candidates, InputPhase);

	// 창이 못 받은 입력은 창 아래 상태가 평소대로 해석한다 (스위치 창 안 공격은 늘 무언가 나간다).
	if ( bHandled == false && WindowRule != nullptr && WindowRule->UnhandledFallThroughInputs.Contains(InputGameplayTag) )
	{
		if ( UYSInputStates* Below = FindStateBelow(InputStateMachineComponent) )
		{
			Below->ProcessInput(InputGameplayTag, InputPhase);
		}
	}

	// 라우팅이 끝난 뒤 닫는다. 먼저 닫으면 상태가 바뀌어 다음 입력부터가 아니라 이 입력까지 영향을 받는다.
	if ( WindowRule != nullptr && InputPhase == EYSInputPhase::Pressed && WindowRule->CloseInputs.Contains(InputGameplayTag) )
	{
		CloseWindow();
	}
}

UYSInputStates* UYSInputStates::FindStateBelow(const UYSInputStateMachineComponent* InputStateMachineComponent) const
{
	const TArray<FName>& Requests = InputStateMachineComponent->InputStateRequests;
	const int32 Index = Requests.Find(StateName);

	if ( Index == INDEX_NONE || Index == 0 )
	{
		return nullptr;
	}

	UYSInputStates* Below = InputStateMachineComponent->FindState(Requests[Index - 1]);
	return ( IsValid(Below) && Below != this ) ? Below : nullptr;
}

void UYSInputStates::CloseWindow() const
{
	UYSAbilitySystemComponent* ASC = OwnerASC.Get();

	if ( IsValid(ASC) == false )
	{
		return;
	}

	const FName WindowState = StateName;

	FGameplayEffectQuery Query;
	Query.CustomMatchDelegate.BindLambda([WindowState](const FActiveGameplayEffect& ActiveEffect)
	{
		const UGameplayEffect* Definition = ActiveEffect.Spec.Def;

		if ( IsValid(Definition) == false )
		{
			return false;
		}

		const UYSInputStateGEComponent* StateComponent = Definition->FindComponent<UYSInputStateGEComponent>();
		return StateComponent != nullptr && StateComponent->TargetToChange == WindowState;
	});

	ASC->RemoveActiveEffects(Query);
}

void UYSInputStates::ResolveStateTags(const FGameplayTag& InputGameplayTag, TArray<FGameplayTag>& OutTags)
{
	if ( const TArray<FGameplayTag>* Cached = ResolvedTagCache.Find(InputGameplayTag) )
	{
		OutTags = *Cached;
		return;
	}

	TArray<FString> Scopes;
	Scopes.Add(StateName.ToString());
	Scopes.Append(Definition.CategoryNames);

	TArray<FGameplayTag> Resolved;

	for ( const FString& Scope : Scopes )
	{
		const FString TagStr = FString::Printf(TEXT("%s.%s"), *Scope, *InputGameplayTag.ToString());
		const FGameplayTag Tag = FGameplayTag::RequestGameplayTag(FName(*TagStr), /*ErrorIfNotFound=*/false);

		if ( Tag.IsValid() )
		{
			Resolved.Add(Tag);
		}
	}

	ResolvedTagCache.Emplace(InputGameplayTag, Resolved);
	OutTags = Resolved;
}

void UYSInputStates::InitState(AActor* Owner, FName InStateName, const FYSInputStateDefinition& InDefinition)
{
	StateName = InStateName;
	Definition = InDefinition;
	OwnerActor = Owner;
	if ( IAbilitySystemInterface* ASI = Cast<IAbilitySystemInterface>(Owner) )
	{
		OwnerASC = Cast<UYSAbilitySystemComponent>(ASI->GetAbilitySystemComponent());
	}
	
	if (UYSInputStateMachineComponent* InputStateMachineComponent = UYSInputStateMachineComponent::Get(Owner))
	{
		OwnerStateMachineManager = InputStateMachineComponent;
	}
}
