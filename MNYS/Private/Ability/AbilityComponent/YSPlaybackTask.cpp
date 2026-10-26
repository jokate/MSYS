// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AbilityComponent/YSPlaybackTask.h"

#include "Abilities/Tasks/AbilityTask.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Abilities/Tasks/AbilityTask_WaitDelay.h"
#include "Abilities/Tasks/AbilityTask_WaitGameplayEvent.h"
#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Ability/MontageSelector/YSMontageSelector.h"
#include "Ability/Task/YSAT_PlaySequence.h"
#include "YSDeveloperSettings.h"
#include "Character/Components/YSCombatTargetComponent.h"
#include "Combat/YSSwitchEntryForm.h"
#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Animation/AnimMontage.h"
#include "GenericTeamAgentInterface.h"
#include "MotionWarpingComponent.h"
#include "TimerManager.h"
#include "YSBattleActor.h"
#include "Ability/Ability/YSGameplayAbility_ContextSwitch.h"
#include "Ability/GameplayEffect/YSGE_ContextSwitchStagger.h"
#include "Character/YSMonster.h"
#include "Combat/YSContextSwitchVariant.h"
#include "Framework/YSGameModeBase.h"
#include "General/YSGameplayTag.h"
#include "Library/YSBlueprintFunctionLibrary.h"
#include "Targeting/YSTargetingShape.h"

#define LOCTEXT_NAMESPACE "YSPlaybackTask"

void UYSPlaybackTask::Start(UYSAbilityPlaybackBase* InOwnerNode)
{
	OwnerNode = InOwnerNode;
	RunningTasks.Reset();
	bRunning = true;
	bStartedWork = true;
	OnStart();
}

void UYSPlaybackTask::Stop()
{
	if ( bRunning == false )
	{
		return;
	}

	// 먼저 내려야 EndTask 도중 되돌아오는 콜백이 전환을 다시 요청하지 못한다.
	bRunning = false;

	for ( UAbilityTask* Task : RunningTasks )
	{
		// 스스로 끝난 Task 는 이미 가비지 표시라 IsValid 에서 걸러진다.
		if ( IsValid(Task) )
		{
			Task->EndTask();
		}
	}

	RunningTasks.Reset();
	OnStop();
}

bool UYSPlaybackTask::TryResolve(int32 OutputIndex)
{
	if ( bRunning == false || IsValid(OwnerNode) == false )
	{
		return false;
	}

	static const TArray<FYSPlaybackRoute> NoRoutes;
	const TArray<FYSPlaybackRoute>& Routes = OutputRoutes.IsValidIndex(OutputIndex) ? OutputRoutes[OutputIndex].Routes : NoRoutes;

	return OwnerNode->TryResolveRoutes(Routes);
}

void UYSPlaybackTask::Resolve(int32 OutputIndex)
{
	if ( bRunning == false || IsValid(OwnerNode) == false )
	{
		return;
	}

	// 경로가 전부 실패하면 체인을 끝낸다. 예전 HandleUnmatchedEvent 와 같은 결과다.
	if ( TryResolve(OutputIndex) == false )
	{
		OwnerNode->RequestTransition(INDEX_NONE);
	}
}

FText UYSPlaybackTask::GetOutputDisplayName(int32 OutputIndex) const
{
	return LOCTEXT("OutputCompleted", "완료");
}

void UYSPlaybackTask::ResetRoutes()
{
	OutputRoutes.Reset();
	OutputRoutes.SetNum(GetOutputCount());
}

void UYSPlaybackTask::AddRoute(int32 OutputIndex, const FYSPlaybackRoute& Route)
{
	if ( OutputIndex < 0 )
	{
		return;
	}

	if ( OutputRoutes.Num() <= OutputIndex )
	{
		OutputRoutes.SetNum(OutputIndex + 1);
	}

	OutputRoutes[OutputIndex].Routes.Add(Route);
}

UYSGameplayAbility* UYSPlaybackTask::GetOwningAbility() const
{
	return IsValid(OwnerNode) ? OwnerNode->GetCurrentPlaybackOwningAbility() : nullptr;
}

TSharedPtr<FYSPlaybackContext> UYSPlaybackTask::GetContext() const
{
	return IsValid(OwnerNode) ? OwnerNode->GetContext() : nullptr;
}

void UYSPlaybackTask_PlayMontage::ReleaseMotionWarp()
{
	const FYSMontageSelector* Selector = MontageSelector.GetPtr<FYSMontageSelector>();
	UYSGameplayAbility* Ability = GetOwningAbility();

	if ( Selector == nullptr || IsValid(Ability) == false )
	{
		return;
	}

	Selector->SetMotionWarp(Ability, false);
}

void UYSPlaybackTask_PlayMontage::OnStart()
{
	const FYSMontageSelector* Selector = MontageSelector.GetPtr<FYSMontageSelector>();
	UYSGameplayAbility* Ability = GetOwningAbility();

	if ( Selector == nullptr || IsValid(Ability) == false )
	{
		MarkWorkNotStarted();
		return;
	}

	UAnimMontage* Montage = Selector->SelectMontage(Ability);
	bool bPlaying = false;

	if ( IsValid(Montage) )
	{
		UAbilityTask_PlayMontageAndWait* Task = Track(UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(Ability, TEXT("PlayMontage"), Montage));

		if ( IsValid(Task) )
		{
			Task->OnCompleted.AddDynamic(this, &ThisClass::OnMontageFinished);
			Task->OnBlendOut.AddDynamic(this, &ThisClass::OnMontageFinished);
			Task->OnInterrupted.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			Task->OnCancelled.AddDynamic(this, &ThisClass::OnMontageInterrupted);
			Task->ReadyForActivation();

			// 재생 실패는 Activate 안에서 동기로 OnCancelled 가 나가 노드가 이미 끝난다. 성공한 재생은 여기서 끝나지 않는다.
			bPlaying = IsRunning();
		}
	}

	if ( bPlaying == false )
	{
		MarkWorkNotStarted();
	}

	// 재생 시작 중 동기로 취소돼 노드가 이미 멈췄으면 워프를 걸지 않는다. 푸는 쪽(OnStop)이 이미 지나갔다.
	if ( IsRunning() )
	{
		Selector->SetMotionWarp(Ability, true);
	}
}

void UYSPlaybackTask_PlayMontage::OnStop()
{
	ReleaseMotionWarp();
}

FText UYSPlaybackTask_PlayMontage::GetOutputDisplayName(int32 OutputIndex) const
{
	return ( OutputIndex == Output_Interrupted )
		? LOCTEXT("OutputInterrupted", "중단")
		: LOCTEXT("OutputCompleted", "완료");
}

void UYSPlaybackTask_PlayMontage::OnMontageFinished()
{
	Resolve(Output_Completed);
}

void UYSPlaybackTask_PlayMontage::OnMontageInterrupted()
{
	Resolve(Output_Interrupted);
}

void UYSPlaybackTask_PlaySequence::OnStart()
{
	UYSGameplayAbility* Ability = GetOwningAbility();
	const TSharedPtr<FYSPlaybackContext> Context = GetContext();

	if ( IsValid(Ability) == false || Context.IsValid() == false )
	{
		return;
	}

	UYSAT_PlaySequence* Task = Track(UYSAT_PlaySequence::CreatePlaySequenceTask(Ability, SequenceSettings, Context->Instigator, Context->Target));

	if ( IsValid(Task) )
	{
		Task->OnSequenceFinishedDelegate.AddDynamic(this, &ThisClass::OnSequenceFinished);
		Task->ReadyForActivation();
	}
}

void UYSPlaybackTask_PlaySequence::OnSequenceFinished()
{
	Resolve();
}

void UYSPlaybackTask_WaitInput::OnStart()
{
	bInputReceived = false;
}

void UYSPlaybackTask_WaitEvent::OnStart()
{
	UAbilityTask_WaitGameplayEvent* Task = Track(UAbilityTask_WaitGameplayEvent::WaitGameplayEvent(GetOwningAbility(), EventTag, nullptr, false));
	
	if ( IsValid(Task) )
	{
		Task->EventReceived.AddDynamic(this, &ThisClass::OnEventReceived);
		Task->ReadyForActivation();
	}
}

FText UYSPlaybackTask_WaitEvent::GetOutputDisplayName(int32 OutputIndex) const
{
	return LOCTEXT("OutputEvent", "이벤트");
}

void UYSPlaybackTask_WaitEvent::OnEventReceived(FGameplayEventData Payload)
{
	Resolve();
}

FText UYSPlaybackTask_WaitInput::GetOutputDisplayName(int32 OutputIndex) const
{
	return LOCTEXT("OutputInput", "입력");
}

bool UYSPlaybackTask_WaitInput::HandleInput(const FGameplayTag& InInputTag, EYSInputPhase InInputPhase)
{
	if ( InputTag.IsValid() == false || InInputTag != InputTag || InInputPhase != InputPhase )
	{
		return false;
	}

	if ( bWaitInputWindowClose )
	{
		bInputReceived = true;
		return true;
	}

	// 경로가 전부 실패하면 false — 노드가 다음 Task 에 기회를 준다.
	return TryResolve();
}

bool UYSPlaybackTask_WaitInput::HandleInputWindowClosed()
{
	if ( bWaitInputWindowClose == false )
	{
		return false;
	}

	if ( InputTag.IsValid() && bInputReceived == false )
	{
		return false;
	}

	// 유지 경로로 끝나면 이 Task 는 계속 대기한다. 다음 창은 새 입력을 받아야 한다.
	bInputReceived = false;

	return TryResolve();
}

void UYSPlaybackTask_WaitDelay::OnStart()
{
	UYSGameplayAbility* Ability = GetOwningAbility();

	if ( IsValid(Ability) == false )
	{
		return;
	}

	UAbilityTask_WaitDelay* Task = Track(UAbilityTask_WaitDelay::WaitDelay(Ability, DelayTime));

	if ( IsValid(Task) )
	{
		Task->OnFinish.AddDynamic(this, &ThisClass::OnDelayFinished);
		Task->ReadyForActivation();
	}
}

void UYSPlaybackTask_WaitDelay::OnDelayFinished()
{
	Resolve();
}

void UYSPlaybackTask_SwitchEntry::OnStart()
{
	SelectedForm = NAME_None;
	bSelected = false;
	bWarpSet = false;

	UAbilityTask_WaitDelay* Task = Track(UAbilityTask_WaitDelay::WaitDelay(GetOwningAbility(), InputWindowLimit));

	if ( IsValid(Task) )
	{
		Task->OnFinish.AddDynamic(this, &ThisClass::OnWindowTimedOut);
		Task->ReadyForActivation();
	}
}

void UYSPlaybackTask_SwitchEntry::OnStop()
{
	ReleaseMotionWarp();
}

FText UYSPlaybackTask_SwitchEntry::GetOutputDisplayName(int32 OutputIndex) const
{
	return LOCTEXT("OutputSwitchEntryHit", "판정");
}

bool UYSPlaybackTask_SwitchEntry::HandleInput(const FGameplayTag& InInputTag, EYSInputPhase InInputPhase)
{
	if ( bSelected || InInputPhase != EYSInputPhase::Pressed )
	{
		return false;
	}

	const FName RowName = GetDefault<UYSDeveloperSettings>()->FindSwitchEntryFormName(InInputTag);

	if ( RowName.IsNone() )
	{
		return false;
	}

	SelectForm(RowName);
	return true;
}

bool UYSPlaybackTask_SwitchEntry::HandleInputWindowClosed()
{
	if ( bSelected )
	{
		return false;
	}

	SelectForm(GetDefault<UYSDeveloperSettings>()->FindSwitchEntryFormName(FGameplayTag()));
	return true;
}

void UYSPlaybackTask_SwitchEntry::OnWindowTimedOut()
{
	HandleInputWindowClosed();
}

void UYSPlaybackTask_SwitchEntry::SelectForm(FName RowName)
{
	bSelected = true;

	const TSharedPtr<FYSPlaybackContext> Context = GetContext();
	const UYSDeveloperSettings* Settings = GetDefault<UYSDeveloperSettings>();

	if ( Context.IsValid() == false )
	{
		return;
	}

	AActor* Instigator = Context->Instigator;
	AActor* Target = FindCombatTarget(Instigator);

	// 조건을 못 채우면 대체 형으로. 표가 서로를 가리켜도 돌지 않게 횟수를 막는다.
	const FYSSwitchEntryForm* Form = Settings->FindSwitchEntryForm(RowName);

	for ( int32 Depth = 0; Form != nullptr && Form->CanUse(Instigator, Target) == false; ++Depth )
	{
		RowName = Depth < 4 ? Form->FallbackForm : NAME_None;
		Form = Settings->FindSwitchEntryForm(RowName);
	}

	if ( Form == nullptr )
	{
		UE_LOG(LogTemp, Warning, TEXT("[SwitchEntry] 쓸 수 있는 진입기 형이 없다 (%s)."), *RowName.ToString());
		return;
	}

	SelectedForm = RowName;
	Context->Target = Target;

	Form->SetWarp(Instigator, Target);
	bWarpSet = true;

	if ( Form->HitDelay <= 0.f )
	{
		ExecuteHit();
		return;
	}

	UAbilityTask_WaitDelay* Task = Track(UAbilityTask_WaitDelay::WaitDelay(GetOwningAbility(), Form->HitDelay));

	if ( IsValid(Task) )
	{
		Task->OnFinish.AddDynamic(this, &ThisClass::OnHitDelayFinished);
		Task->ReadyForActivation();
	}
}

void UYSPlaybackTask_SwitchEntry::OnHitDelayFinished()
{
	ExecuteHit();
}

void UYSPlaybackTask_SwitchEntry::ExecuteHit()
{
	const FYSSwitchEntryForm* Form = GetDefault<UYSDeveloperSettings>()->FindSwitchEntryForm(SelectedForm);
	const TSharedPtr<FYSPlaybackContext> Context = GetContext();

	if ( Form == nullptr || Context.IsValid() == false )
	{
		return;
	}

	TArray<AActor*> Targets;
	Form->CollectTargets(Context->Instigator, Context->Target, Targets);
	Form->ApplyToTargets(Context->Instigator, Targets);

	// 경로가 없으면 전환하지 않는다. 노드는 몽타주가 끝낸다.
	TryResolve();
}

void UYSPlaybackTask_SwitchEntry::ReleaseMotionWarp()
{
	if ( bWarpSet == false )
	{
		return;
	}

	bWarpSet = false;

	const TSharedPtr<FYSPlaybackContext> Context = GetContext();

	if ( const FYSSwitchEntryForm* Form = GetDefault<UYSDeveloperSettings>()->FindSwitchEntryForm(SelectedForm) )
	{
		Form->ReleaseWarp(Context.IsValid() ? Context->Instigator.Get() : nullptr);
	}
}

AActor* UYSPlaybackTask_SwitchEntry::FindCombatTarget(const AActor* Instigator) const
{
	const UYSCombatTargetComponent* CombatTarget = UYSCombatTargetComponent::Get(Instigator);

	return IsValid(CombatTarget) ? CombatTarget->GetCurrentTarget() : nullptr;
}

const FYSContextSwitchVariant* UYSPlaybackTask_ContextSwitchHit::GetVariant() const
{
	const UYSGameplayAbility_ContextSwitch* Ability = Cast<UYSGameplayAbility_ContextSwitch>(GetOwningAbility());
	return IsValid(Ability) ? Ability->GetSelectedVariant() : nullptr;
}

void UYSPlaybackTask_ContextSwitchHit::OnStart()
{
	bWarpSet = false;

	const UYSGameplayAbility_ContextSwitch* Ability = Cast<UYSGameplayAbility_ContextSwitch>(GetOwningAbility());
	const FYSContextSwitchVariant* Variant = GetVariant();
	const TSharedPtr<FYSPlaybackContext> Context = GetContext();

	if ( Variant == nullptr || Context.IsValid() == false )
	{
		UE_LOG(LogTemp, Warning, TEXT("[ContextSwitch] 변형 행이 없다. 이 태스크는 UYSGameplayAbility_ContextSwitch 플레이백에만 둔다."));
		return;
	}

	AActor* Instigator = Context->Instigator;
	AActor* Target = Ability->GetVariantTarget();
	Context->Target = Target;

	// 회전은 제자리에서 타겟 쪽으로, 워프는 타겟 앞까지. 돌진은 몽타주 루트모션 그대로다.
	UMotionWarpingComponent* MotionWarping = IsValid(Instigator) ? Instigator->FindComponentByClass<UMotionWarpingComponent>() : nullptr;

	if ( Variant->Move != EYSContextSwitchMove::Dash && Variant->WarpTargetName.IsNone() == false && IsValid(MotionWarping) && IsValid(Target) )
	{
		const FVector From = Instigator->GetActorLocation();
		FVector ToTarget = Target->GetActorLocation() - From;
		ToTarget.Z = 0.f;

		const FVector Direction = ToTarget.GetSafeNormal();
		const float Travel = ( Variant->Move == EYSContextSwitchMove::Warp )
			? FMath::Clamp(ToTarget.Size() - Variant->WarpStopDistance, 0.f, Variant->WarpMaxDistance)
			: 0.f;

		MotionWarping->AddOrUpdateWarpTargetFromLocationAndRotation(Variant->WarpTargetName, From + Direction * Travel, Direction.Rotation());
		bWarpSet = true;
	}

	float HitDelay = Variant->HitDelay;

	// 블렌드아웃이 체인을 끝내면 이 대기도 정리돼 판정이 사라진다. 그보다 늦은 판정은 블렌드아웃 직전으로 당긴다.
	const UAnimMontage* Montage = Variant->Montage.LoadSynchronous();

	if ( IsValid(Montage) && Montage->bEnableAutoBlendOut )
	{
		constexpr float HitLeadBeforeBlendOut = 0.05f;

		// 엔진은 남은 재생 시간(실초)이 블렌드아웃 시간(실초) 이하가 될 때 블렌드아웃을 시작한다. 배율은 몽타주 길이에만 걸린다.
		const float FromEnd = ( Montage->BlendOutTriggerTime >= 0.f ) ? Montage->BlendOutTriggerTime : Montage->GetDefaultBlendOutTime();
		const float BlendOutStart = Montage->GetPlayLength() / FMath::Max(Montage->RateScale, UE_KINDA_SMALL_NUMBER) - FromEnd;

		// 당겨도 0 이 되지 않게 한다. 0 이면 회전 · 워프 전에 판정한다. 하한이 블렌드아웃 시작을 넘으면 판정이 사라지므로 그 안에서만 둔다.
		const float MinHit = FMath::Min(FMath::Min(HitDelay, HitLeadBeforeBlendOut), FMath::Max(BlendOutStart, 0.f));
		const float LatestHit = FMath::Max(MinHit, BlendOutStart - HitLeadBeforeBlendOut);

		if ( HitDelay > LatestHit )
		{
			UE_LOG(LogTemp, Warning, TEXT("[ContextSwitch] 변형 행 '%s' 의 판정 지연 %.2f초가 몽타주 블렌드아웃 시작(%.2f초)보다 늦어 %.2f초로 당긴다."),
				*Ability->GetSelectedRow().ToString(), HitDelay, BlendOutStart, LatestHit);
			HitDelay = LatestHit;
		}
	}

	if ( HitDelay <= 0.f )
	{
		ExecuteHit();
		return;
	}

	UAbilityTask_WaitDelay* Task = Track(UAbilityTask_WaitDelay::WaitDelay(GetOwningAbility(), HitDelay));

	if ( IsValid(Task) )
	{
		Task->OnFinish.AddDynamic(this, &ThisClass::OnHitDelayFinished);
		Task->ReadyForActivation();
	}
}

void UYSPlaybackTask_ContextSwitchHit::OnStop()
{
	ReleaseMotionWarp();
}

FText UYSPlaybackTask_ContextSwitchHit::GetOutputDisplayName(int32 OutputIndex) const
{
	return LOCTEXT("OutputContextSwitchHit", "판정");
}

void UYSPlaybackTask_ContextSwitchHit::OnHitDelayFinished()
{
	ExecuteHit();
}

void UYSPlaybackTask_ContextSwitchHit::ExecuteHit()
{
	const UYSGameplayAbility_ContextSwitch* Ability = Cast<UYSGameplayAbility_ContextSwitch>(GetOwningAbility());
	const FYSContextSwitchVariant* Variant = GetVariant();
	const TSharedPtr<FYSPlaybackContext> Context = GetContext();
	const FYSTargetingShape* HitShape = Variant ? Variant->Shape.GetPtr<FYSTargetingShape>() : nullptr;
	AActor* Instigator = Context.IsValid() ? Context->Instigator.Get() : nullptr;

	if ( HitShape == nullptr || IsValid(Instigator) == false )
	{
		TryResolve();
		return;
	}

	TArray<AActor*> InShape;

	if ( HitShape->CollectActors(Instigator->GetWorld(), Instigator->GetActorLocation(), Instigator->GetActorForwardVector(), Instigator, InShape) == false )
	{
		UE_LOG(LogTemp, Warning, TEXT("[ContextSwitch] 판정 도형이 판정을 지원하지 않는다 (시전자 기준 도형만 된다)."));
	}

	AActor* VariantTarget = Ability->GetVariantTarget();

	// 1체 규칙은 관통 없이 시작 때 고른 타겟만.
	if ( Variant->TargetRule == EYSSwitchEntryTargetRule::CurrentTarget )
	{
		InShape = InShape.Contains(VariantTarget) ? TArray<AActor*>{ VariantTarget } : TArray<AActor*>();
	}

	UAbilitySystemComponent* SourceASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Instigator);
	const float Scale = Ability->GetEffectScale();
	TArray<AActor*> HitTargets;

	// 판정은 한 번이라 이번 판정 안에서만 저스트 회피 GE 를 대상당 한 번 준다.
	TSet<TWeakObjectPtr<AActor>> JustAvoidGranted;

	for ( AActor* Target : InShape )
	{
		// 아군 · 사망 · 저스트 회피 창 · 무적 검사는 트레이스 · DirectHit 과 같은 함수를 거친다.
		if ( UYSBlueprintFunctionLibrary::AdmitHit(Instigator, Target, &JustAvoidGranted) == false )
		{
			continue;
		}

		UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(Target);

		// 등급 0 · 1 은 그로기 게이지가 없다. 그로기 피해 대신 경직 종류(피해 행)와 경직 GE 를 받는다.
		const AYSMonster* Monster = Cast<AYSMonster>(Target);
		const bool bLightGrade = IsValid(Monster) && Monster->StaggerGrade != EYSStaggerGrade::Immovable;
		const FName Row = ( bLightGrade && Variant->StaggerDamageRow.IsNone() == false ) ? Variant->StaggerDamageRow : Variant->DamageRow;

		if ( Row.IsNone() == false )
		{
			const FVector HitLocation = Target->GetActorLocation();
			const FVector HitDirection = ( HitLocation - Instigator->GetActorLocation() ).GetSafeNormal2D();
			const FHitResult HitResult(Target, Cast<UPrimitiveComponent>(Target->GetRootComponent()), HitLocation, -HitDirection);

			UYSBlueprintFunctionLibrary::SendHitEventToTarget(Instigator, Target, Row, Scale);
			UYSBlueprintFunctionLibrary::SpawnEffects(Instigator, Row, HitLocation, HitDirection.ToOrientationRotator());
			UYSBlueprintFunctionLibrary::ApplyHitEffects(Instigator, Instigator, Target, Row, HitResult);
		}

		if ( bLightGrade && Variant->StaggerDuration > 0.f && IsValid(SourceASC) )
		{
			const float Duration = Variant->StaggerDuration * Scale;

			FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
			EffectContext.AddInstigator(Instigator, Instigator);

			FGameplayEffectSpec Spec(GetDefault<UYSGE_ContextSwitchStagger>(), EffectContext);
			Spec.SetSetByCallerMagnitude(UYSGE_ContextSwitchStagger::DurationName, Duration);
			// SetByCaller 로 정한 시간은 스펙 생성 때 계산되지 않아 다시 잡는다.
			Spec.SetDuration(Duration, true);
			Spec.DynamicGrantedTags.AddTag(YSTags::AISuppress_HitReact);

			SourceASC->ApplyGameplayEffectSpecToTarget(Spec, TargetASC);
		}

		HitTargets.Add(Target);
	}

	// 변형 행의 임팩트 칸이 등급이다 (HUD 보고와 같은 칸). 공격 1회에 한 번.
	if ( HitTargets.Num() > 0 )
	{
		UYSBlueprintFunctionLibrary::PlayImpact(Instigator, HitTargets, Variant->Impact);
	}

	// 경로가 없으면 전환하지 않는다. 노드는 몽타주가 끝낸다.
	TryResolve();
}

void UYSPlaybackTask_ContextSwitchHit::ReleaseMotionWarp()
{
	if ( bWarpSet == false )
	{
		return;
	}

	bWarpSet = false;

	const FYSContextSwitchVariant* Variant = GetVariant();
	const TSharedPtr<FYSPlaybackContext> Context = GetContext();
	AActor* Instigator = Context.IsValid() ? Context->Instigator.Get() : nullptr;

	if ( Variant == nullptr || IsValid(Instigator) == false )
	{
		return;
	}

	if ( UMotionWarpingComponent* MotionWarping = Instigator->FindComponentByClass<UMotionWarpingComponent>() )
	{
		MotionWarping->RemoveWarpTarget(Variant->WarpTargetName);
	}
}

#undef LOCTEXT_NAMESPACE
