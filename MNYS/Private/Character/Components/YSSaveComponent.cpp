// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/YSSaveComponent.h"

#include "YSAbilitySystemComponent.h"
#include "Ability/YSGameplayAbility.h"
#include "AttackableActor/YSSaveEcho.h"
#include "General/YSGameplayTag.h"
#include "General/YSInputGameplayTags.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"
#include "Library/YSBlueprintFunctionLibrary.h"


bool FYSSavedTechnique::IsValid() const
{
	return AbilityClass != nullptr && PlaybackIndex != INDEX_NONE;
}

void FYSSavedTechnique::Reset()
{
	AbilityClass = nullptr;
	PlaybackIndex = INDEX_NONE;
	SavedMultiplier = 1.f;
}


UYSSaveComponent* UYSSaveComponent::Get(AActor* Owner)
{
	if ( IsValid(Owner) == false )
	{
		return nullptr;
	}

	return Owner->FindComponentByClass<UYSSaveComponent>();
}

UYSSaveComponent::UYSSaveComponent()
{
	// 슬롯은 사건이 있을 때만 바뀐다. 매 프레임 볼 것이 없다.
	PrimaryComponentTick.bCanEverTick = false;
}

void UYSSaveComponent::BeginPlay()
{
	Super::BeginPlay();

	SavedTechniques.SetNum(MaxSlotCount);
	RefreshStateTags();

	// 세이브는 어빌리티가 아니다. 활성 콤보가 F 를 받아 체인을 끊지 않게 소비 등록한다.
	if ( UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(GetOwner()) )
	{
		InputStateMachine->OnRawInputAccepted.AddUniqueDynamic(this, &UYSSaveComponent::HandleRawInput);
		InputStateMachine->ClaimRawInput(YSInputTags::InputSave, this);
	}

	if ( UYSAbilitySystemComponent* ASC = GetOwnerASC() )
	{
		ASC->OnGameplayTagStateChanged.AddUniqueDynamic(this, &UYSSaveComponent::HandleTagStateChanged);
	}

	RefreshStateTags();
}

bool UYSSaveComponent::HasSavedTechnique() const
{
	for (const FYSSavedTechniqueSlot& Slot : SavedTechniques)
	{
		if ( Slot.SavedTechnique.IsValid() )
		{
			return true;
		}
	}
	
	return false;
}

bool UYSSaveComponent::IsSlotFull() const
{
	for ( const FYSSavedTechniqueSlot& Slot : SavedTechniques )
	{
		if ( Slot.SavedTechnique.IsValid() == false )
		{
			return false;
		}
	}
	
	return true;
}

void UYSSaveComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if ( UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(GetOwner()) )
	{
		InputStateMachine->OnRawInputAccepted.RemoveDynamic(this, &UYSSaveComponent::HandleRawInput);
		InputStateMachine->ReleaseRawInputs(this);
	}

	if ( UYSAbilitySystemComponent* ASC = GetOwnerASC() )
	{
		ASC->OnGameplayTagStateChanged.RemoveDynamic(this, &UYSSaveComponent::HandleTagStateChanged);
	}

	QueuedEchoTechniques.Empty();

	Super::EndPlay(EndPlayReason);
}

void UYSSaveComponent::MarkSavable(TSubclassOf<UYSGameplayAbility> AbilityClass, int32 PlaybackIndex, float Multiplier)
{
	if ( AbilityClass == nullptr || PlaybackIndex == INDEX_NONE )
	{
		return;
	}

	UWorld* World = GetWorld();

	if ( IsValid(World) == false )
	{
		return;
	}

	// 최신 거로 덮어쓰긴 할 거임.
	PendingTechnique = FYSSavedTechnique(AbilityClass, PlaybackIndex, Multiplier);
	RefreshStateTags();
	OnSaveStateChanged.Broadcast();
}

bool UYSSaveComponent::TryCommitPending()
{
	if ( PendingTechnique.IsValid() == false )
	{
		return false;
	}

	// 찼으면 저장 데이터를 소진하지 않는 이상 절대 소모 불가.
	if ( IsSlotFull() )
	{
		return false;
	}

	bool bIsSavedComplete = false;
	for ( FYSSavedTechniqueSlot& Slot : SavedTechniques )
	{
		
		bool bTimeCondition = Slot.LastUsedTime == 0 || (GetWorld()->GetTimeSeconds() - Slot.LastUsedTime) >= SaveCooldown;
		if ( Slot.SavedTechnique.IsValid() == false && bTimeCondition )
		{
			Slot.SavedTechnique = PendingTechnique;
			bIsSavedComplete = true;
			break;
		}
	}
	
	if (!bIsSavedComplete)
		return false;
	
	RefreshStateTags();
	OnSaveStateChanged.Broadcast();

	return true;
}

bool UYSSaveComponent::TryConsumeSlot(FYSSavedTechnique& OutTechnique)
{
	if ( HasSavedTechnique() == false )
	{
		return false;
	}
	
	for ( FYSSavedTechniqueSlot& Slot : SavedTechniques )
	{
		if ( Slot.SavedTechnique.IsValid() )
		{
			OutTechnique = Slot.SavedTechnique;
			Slot.SavedTechnique.Reset();
			Slot.LastUsedTime = GetWorld()->GetTimeSeconds();
			break;
		}
	}

	RefreshStateTags();
	OnSaveStateChanged.Broadcast();

	return true;
}

bool UYSSaveComponent::TakeEchoTechnique(const TSharedPtr<FYSAbilityHitContext>& HitContext, FYSSavedTechnique& OutTechnique)
{
	// 대기 스폰이 히트 기록을 쥐고 있으므로, 기록이 사라진 항목은 스폰이 오지 않는다.
	QueuedEchoTechniques.RemoveAll([](const TPair<FYSSavedTechnique, TWeakPtr<FYSAbilityHitContext>>& Queued)
	{
		return Queued.Value.IsValid() == false;
	});

	const int32 QueuedIndex = HitContext.IsValid() == false ? INDEX_NONE : QueuedEchoTechniques.IndexOfByPredicate(
		[&HitContext](const TPair<FYSSavedTechnique, TWeakPtr<FYSAbilityHitContext>>& Queued)
		{
			return Queued.Value.Pin() == HitContext;
		});

	if ( QueuedIndex == INDEX_NONE )
	{
		return TryConsumeSlot(OutTechnique);
	}

	const FYSSavedTechnique Technique = QueuedEchoTechniques[QueuedIndex].Key;
	QueuedEchoTechniques.RemoveAt(QueuedIndex);

	// 궁극기가 끝나 취소된 참격은 슬롯으로 넘어가지 않는다.
	if ( Technique.IsValid() == false )
	{
		return false;
	}

	OutTechnique = Technique;
	return true;
}

void UYSSaveComponent::ExecuteTripleEcho(const FYSSavedTechnique& Technique, const TSharedPtr<FYSAbilityHitContext>& HitContext)
{
	AActor* Owner = GetOwner();
	if ( IsValid(Owner) == false || Technique.IsValid() == false || HitContext.IsValid() == false || TripleEchoSpawnConfig.ActorClass == nullptr )
	{
		return;
	}

	QueuedEchoTechniques.Emplace(Technique, HitContext);
	UYSBlueprintFunctionLibrary::SpawnByConfig(this, TripleEchoSpawnConfig, FYSTransformPolicyContext(Owner), nullptr, HitContext);
}

void UYSSaveComponent::CancelPendingEchoes()
{
	// 지우지 않고 비워 둔다. 스폰 타이머는 그대로 돌아 분신이 와서 찾으므로, 없으면 슬롯을 꺼내 버린다.
	for ( TPair<FYSSavedTechnique, TWeakPtr<FYSAbilityHitContext>>& Queued : QueuedEchoTechniques )
	{
		Queued.Key.Reset();
	}
}

void UYSSaveComponent::HandleTagStateChanged(const FGameplayTag& Tag, bool bTagExists)
{
	if ( Tag != YSTags::State_Save_TripleEcho || bTagExists )
	{
		return;
	}

	CancelPendingEchoes();
}

void UYSSaveComponent::SetMaxSlotCount(int32 NewMax)
{
	MaxSlotCount = FMath::Max(1, NewMax);
	
	while ( SavedTechniques.Num() > MaxSlotCount )
	{
		SavedTechniques.RemoveAt(0);
	}

	RefreshStateTags();
	OnSaveStateChanged.Broadcast();
}

void UYSSaveComponent::HandleRawInput(const FGameplayTag& InputTag, EYSInputPhase InputPhase)
{
	if ( InputTag != YSInputTags::InputSave || InputPhase != EYSInputPhase::Pressed )
	{
		return;
	}

	TryCommitPending();
}

void UYSSaveComponent::ClearPending()
{
	if ( PendingTechnique.IsValid() == false )
	{
		return;
	}

	PendingTechnique.Reset();

	RefreshStateTags();
	OnSaveStateChanged.Broadcast();
}

void UYSSaveComponent::RefreshStateTags()
{
	UYSAbilitySystemComponent* ASC = GetOwnerASC();

	if ( IsValid(ASC) == false )
	{
		return;
	}
	
	ASC->SetLooseGameplayTagCount(YSTags::State_Save_HasSlot, HasSavedTechnique() ? 1 : 0);
}

UYSAbilitySystemComponent* UYSSaveComponent::GetOwnerASC() const
{
	return UYSAbilitySystemComponent::Get(GetOwner());
}
