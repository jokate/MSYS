// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/Components/YSCombatTargetComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GenericTeamAgentInterface.h"
#include "TimerManager.h"
#include "YSDeveloperSettings.h"
#include "Character/AttributeSet/YSCharacterAttributeSetBase.h"
#include "Data/YSCombatProfile.h"
#include "Data/YSDataStruct.h"
#include "Engine/OverlapResult.h"
#include "Engine/World.h"
#include "GameFramework/PlayerController.h"
#include "General/YSDefine.h"
#include "General/YSGameplayTag.h"
#include "General/YSInputGameplayTags.h"
#include "Input/StateMachine/YSInputStateMachineComponent.h"
#include "Subsystem/YSSquadSubsystem.h"


UYSCombatTargetComponent::UYSCombatTargetComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

UYSCombatTargetComponent* UYSCombatTargetComponent::Get(const AActor* PawnOrController)
{
	if ( IsValid(PawnOrController) == false )
	{
		return nullptr;
	}

	if ( const APawn* Pawn = Cast<APawn>(PawnOrController) )
	{
		const AController* Controller = Pawn->GetController();
		return IsValid(Controller) ? Controller->FindComponentByClass<UYSCombatTargetComponent>() : nullptr;
	}

	return PawnOrController->FindComponentByClass<UYSCombatTargetComponent>();
}

void UYSCombatTargetComponent::BeginPlay()
{
	Super::BeginPlay();

	SetComponentTickInterval(MaintainInterval);

	if ( APlayerController* PC = Cast<APlayerController>(GetOwner()) )
	{
		PC->OnPossessedPawnChanged.AddDynamic(this, &UYSCombatTargetComponent::HandlePossessedPawnChanged);
		BindPawn(PC->GetPawn());
	}

	OnTargetChanged.AddDynamic(this, &UYSCombatTargetComponent::HandleOwnTargetChanged);
}

void UYSCombatTargetComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	UnbindPawn();
	StopEngageRetry();
	OnTargetChanged.RemoveDynamic(this, &UYSCombatTargetComponent::HandleOwnTargetChanged);

	Super::EndPlay(EndPlayReason);
}

void UYSCombatTargetComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	ScanNearbyEnemies();
	MaintainTarget();
}

void UYSCombatTargetComponent::HandlePossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	UnbindPawn();
	BindPawn(NewPawn);
}

void UYSCombatTargetComponent::BindPawn(APawn* Pawn)
{
	ResolveProfile();

	if ( IsValid(Pawn) == false )
	{
		return;
	}

	// 타겟 전환은 어빌리티가 아니다. 상태 해석을 거치지 않는 원본 입력 통로로 받고,
	// 활성 콤보가 Q/E 를 받아 체인을 끊지 않게 소비 등록한다.
	if ( UYSInputStateMachineComponent* InputStateMachine = UYSInputStateMachineComponent::Get(Pawn) )
	{
		InputStateMachine->OnRawInputAccepted.AddUniqueDynamic(this, &UYSCombatTargetComponent::HandleRawInput);
		InputStateMachine->ClaimRawInput(YSInputTags::InputTargetNext, this);
		InputStateMachine->ClaimRawInput(YSInputTags::InputTargetPrev, this);
		BoundInputStateMachine = InputStateMachine;
	}
}

void UYSCombatTargetComponent::UnbindPawn()
{
	if ( UYSInputStateMachineComponent* InputStateMachine = BoundInputStateMachine.Get() )
	{
		InputStateMachine->OnRawInputAccepted.RemoveDynamic(this, &UYSCombatTargetComponent::HandleRawInput);
		InputStateMachine->ReleaseRawInputs(this);
	}

	BoundInputStateMachine.Reset();
}

void UYSCombatTargetComponent::ResolveProfile()
{
	ActiveProfile.Reset();
	LoadedDefaultProfile = nullptr;

	// 지금 필드 캐릭터의 정의에서 찾는다. 스쿼드 없이 띄운 맵이면 설정의 기본값으로 간다.
	if ( const UGameInstance* GameInstance = GetWorld() ? GetWorld()->GetGameInstance() : nullptr )
	{
		if ( const UYSSquadSubsystem* Squad = GameInstance->GetSubsystem<UYSSquadSubsystem>() )
		{
			FYSCharacterInfo Definition;
			UYSAbilitySystemComponent* UnusedASC = nullptr;

			if ( Squad->GetSlotInfo(Squad->GetActiveSlot(), Definition, UnusedASC) && IsValid(Definition.CombatProfile) )
			{
				ActiveProfile = Definition.CombatProfile.Get();
				return;
			}
		}
	}

	LoadedDefaultProfile = GetDefault<UYSDeveloperSettings>()->DefaultCombatProfile.LoadSynchronous();
	ActiveProfile = LoadedDefaultProfile;

	if ( ActiveProfile.IsValid() )
	{
		return;
	}

	// 둘 다 비면 후보 0 · 구간 밖으로 조용히 망가지므로 클래스 기본값(임시값) 프로필로 대신한다.
	ActiveProfile = GetDefault<UYSCombatProfile>();

	UE_CLOG(bWarnedMissingProfile == false, LogTemp, Warning,
		TEXT("[CombatTarget] 전투 프로필이 없어 내장 기본값으로 대신한다. 캐릭터 정의나 YS Developer Settings 의 기본 전투 프로필을 채워야 한다."));
	bWarnedMissingProfile = true;
}

void UYSCombatTargetComponent::HandleRawInput(const FGameplayTag& InputTag, EYSInputPhase InputPhase)
{
	if ( InputPhase != EYSInputPhase::Pressed )
	{
		return;
	}

	if ( InputTag == YSInputTags::InputTargetNext )
	{
		CycleTarget(true);
	}
	else if ( InputTag == YSInputTags::InputTargetPrev )
	{
		CycleTarget(false);
	}
}

APawn* UYSCombatTargetComponent::GetOwnerPawn() const
{
	const AController* Controller = Cast<AController>(GetOwner());
	return IsValid(Controller) ? Controller->GetPawn() : nullptr;
}

bool UYSCombatTargetComponent::GetViewPoint(FVector& OutLocation, FRotator& OutRotation) const
{
	const AController* Controller = Cast<AController>(GetOwner());

	if ( IsValid(Controller) == false )
	{
		return false;
	}

	Controller->GetPlayerViewPoint(OutLocation, OutRotation);
	return true;
}

bool UYSCombatTargetComponent::IsJustAvoidWindowActive() const
{
	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetOwnerPawn());
	return IsValid(ASC) && ASC->HasMatchingGameplayTag(YSTags::JustAvoid_Window);
}

void UYSCombatTargetComponent::GatherCandidates(TArray<AActor*>& OutCandidates) const
{
	OutCandidates.Reset();

	const APawn* Pawn = GetOwnerPawn();
	const UYSCombatProfile* Profile = ActiveProfile.Get();

	if ( IsValid(Pawn) == false || IsValid(Profile) == false )
	{
		return;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(YSCombatTargetGather), false, Pawn);
	TArray<FOverlapResult> Overlaps;

	GetWorld()->OverlapMultiByObjectType(Overlaps, Pawn->GetActorLocation(), FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(Profile->AttackRange), Params);

	for ( const FOverlapResult& Overlap : Overlaps )
	{
		AActor* Candidate = Overlap.GetActor();

		if ( IsValid(Candidate) == false || OutCandidates.Contains(Candidate) )
		{
			continue;
		}

		if ( FGenericTeamId::GetAttitude(Pawn, Candidate) != ETeamAttitude::Hostile )
		{
			continue;
		}

		if ( IsValidCandidate(Candidate) )
		{
			OutCandidates.Add(Candidate);
		}
	}
}

bool UYSCombatTargetComponent::IsExcluded(const AActor* Candidate) const
{
	const UYSCombatProfile* Profile = ActiveProfile.Get();
	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(Candidate));

	// 사망은 데이터와 무관하게 항상 제외한다. AYSCharacterBase::IsDead 와 같은 기준(Hp <= 0)이다.
	if ( IsValid(ASC) )
	{
		if ( const UYSCharacterAttributeSetBase* AttributeSet = ASC->GetSet<UYSCharacterAttributeSetBase>() )
		{
			if ( AttributeSet->GetHp() <= 0.f )
			{
				return true;
			}
		}
	}

	return IsValid(Profile) && IsValid(ASC) && ASC->HasAnyMatchingGameplayTags(Profile->ExcludedTags);
}

bool UYSCombatTargetComponent::IsValidCandidate(const AActor* Candidate) const
{
	const APawn* Pawn = GetOwnerPawn();
	const UYSCombatProfile* Profile = ActiveProfile.Get();

	if ( IsValid(Candidate) == false || IsValid(Pawn) == false || IsValid(Profile) == false )
	{
		return false;
	}

	const FVector TargetLocation = Candidate->GetActorLocation();

	if ( FVector::DistSquared(Pawn->GetActorLocation(), TargetLocation) > FMath::Square(Profile->AttackRange) )
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	if ( GetViewPoint(ViewLocation, ViewRotation) )
	{
		const FVector ToTarget = ( TargetLocation - ViewLocation ).GetSafeNormal();
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ViewRotation.Vector(), ToTarget)));

		if ( Angle > Profile->ViewAngle * 0.5f )
		{
			return false;
		}
	}

	if ( IsExcluded(Candidate) )
	{
		return false;
	}

	// 가림: 폰 눈높이에서 대상까지 막히면 후보가 아니다.
	FCollisionQueryParams Params(SCENE_QUERY_STAT(YSCombatTargetSight), false, Pawn);
	Params.AddIgnoredActor(Candidate);

	FHitResult Hit;
	return GetWorld()->LineTraceSingleByChannel(Hit, Pawn->GetPawnViewLocation(), TargetLocation, ECC_Visibility, Params) == false;
}

float UYSCombatTargetComponent::ScoreCandidate(const AActor* Candidate) const
{
	const APawn* Pawn = GetOwnerPawn();
	const UYSCombatProfile* Profile = ActiveProfile.Get();

	if ( IsValid(Pawn) == false || IsValid(Profile) == false )
	{
		return 0.f;
	}

	// 각 축을 0~1 로 편 뒤 가중치를 곱해 더한다.
	float AngleScore = 0.f;
	FVector ViewLocation;
	FRotator ViewRotation;

	if ( GetViewPoint(ViewLocation, ViewRotation) && Profile->ViewAngle > 0.f )
	{
		const FVector ToTarget = ( Candidate->GetActorLocation() - ViewLocation ).GetSafeNormal();
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ViewRotation.Vector(), ToTarget)));
		AngleScore = 1.f - FMath::Clamp(Angle / ( Profile->ViewAngle * 0.5f ), 0.f, 1.f);
	}

	const float Distance = FVector::Dist(Pawn->GetActorLocation(), Candidate->GetActorLocation());
	const float DistanceScore = Profile->AttackRange > 0.f ? 1.f - FMath::Clamp(Distance / Profile->AttackRange, 0.f, 1.f) : 0.f;

	const float KeepScore = ( CurrentTarget.Get() == Candidate ) ? 1.f : 0.f;

	const UAbilitySystemComponent* ASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(const_cast<AActor*>(Candidate));
	const float ThreatScore = ( IsValid(ASC) && ASC->HasAnyMatchingGameplayTags(Profile->ThreatTags) ) ? 1.f : 0.f;

	return AngleScore * Profile->AngleWeight
		 + DistanceScore * Profile->DistanceWeight
		 + KeepScore * Profile->KeepWeight
		 + ThreatScore * Profile->ThreatWeight;
}

AActor* UYSCombatTargetComponent::SelectBest(const TArray<AActor*>& Candidates) const
{
	const APawn* Pawn = GetOwnerPawn();

	if ( IsValid(Pawn) == false )
	{
		return nullptr;
	}

	AActor* Best = nullptr;
	float BestScore = -BIG_NUMBER;
	float BestDistSq = BIG_NUMBER;

	for ( AActor* Candidate : Candidates )
	{
		const float Score = ScoreCandidate(Candidate);
		const float DistSq = FVector::DistSquared(Pawn->GetActorLocation(), Candidate->GetActorLocation());

		// 동점이면 가까운 쪽, 그래도 같으면 직전 타겟(§5).
		const bool bBetter = Score > BestScore + KINDA_SMALL_NUMBER
			|| ( FMath::IsNearlyEqual(Score, BestScore) && ( DistSq < BestDistSq || ( FMath::IsNearlyEqual(DistSq, BestDistSq) && Candidate == CurrentTarget.Get() ) ) );

		if ( bBetter )
		{
			Best = Candidate;
			BestScore = Score;
			BestDistSq = DistSq;
		}
	}

	return Best;
}

void UYSCombatTargetComponent::SetTarget(AActor* NewTarget, EYSTargetChangeReason Reason)
{
	AActor* OldTarget = CurrentTarget.Get();
	LostSinceTime = -1.f;

	if ( OldTarget == NewTarget )
	{
		return;
	}

	UE_LOG(LogYSMeasure, Verbose, TEXT("[CombatTarget] t=%.3f Old=%s New=%s Reason=%d"),
		GetWorld()->GetTimeSeconds(), *GetNameSafe(OldTarget), *GetNameSafe(NewTarget), static_cast<int32>(Reason));

	CurrentTarget = NewTarget;
	bHasTarget = IsValid(NewTarget);
	bHasLastBand = false;

	OnTargetChanged.Broadcast(OldTarget, NewTarget, Reason);
}

AActor* UYSCombatTargetComponent::RefreshTarget()
{
	if ( IsJustAvoidWindowActive() )
	{
		return CurrentTarget.Get();
	}

	TArray<AActor*> Candidates;
	GatherCandidates(Candidates);
	SetTarget(SelectBest(Candidates), EYSTargetChangeReason::Auto);

	return CurrentTarget.Get();
}

bool UYSCombatTargetComponent::CycleTarget(bool bNext)
{
	// 저스트 파생 대상은 이 규칙이 덮어쓰지 않는다(§1 예외).
	if ( IsJustAvoidWindowActive() )
	{
		return false;
	}

	const APlayerController* PC = Cast<APlayerController>(GetOwner());

	if ( IsValid(PC) == false )
	{
		return false;
	}

	TArray<AActor*> Candidates;
	GatherCandidates(Candidates);

	AActor* Current = CurrentTarget.Get();

	if ( Candidates.Num() == 0 || ( Candidates.Num() == 1 && Candidates[0] == Current ) )
	{
		return false;
	}

	// 화면 왼쪽 → 오른쪽. 화면 밖으로 투영되지 않는 후보는 시야각에서 이미 걸러졌다.
	TArray<TPair<float, AActor*>> Ordered;
	Ordered.Reserve(Candidates.Num());

	for ( AActor* Candidate : Candidates )
	{
		FVector2D ScreenPosition = FVector2D::ZeroVector;
		PC->ProjectWorldLocationToScreen(Candidate->GetActorLocation(), ScreenPosition);
		Ordered.Emplace(ScreenPosition.X, Candidate);
	}

	Ordered.Sort([](const TPair<float, AActor*>& A, const TPair<float, AActor*>& B) { return A.Key < B.Key; });

	const int32 CurrentIndex = Ordered.IndexOfByPredicate([Current](const TPair<float, AActor*>& Entry) { return Entry.Value == Current; });
	const int32 Count = Ordered.Num();

	int32 NextIndex;

	if ( CurrentIndex == INDEX_NONE )
	{
		NextIndex = bNext ? 0 : Count - 1;
	}
	else
	{
		NextIndex = ( CurrentIndex + ( bNext ? 1 : -1 ) + Count ) % Count;
	}

	AActor* NewTarget = Ordered[NextIndex].Value;

	const APawn* Pawn = GetOwnerPawn();
	ManualSwitchSnapshot.Target = NewTarget;
	ManualSwitchSnapshot.PreSwitchForward = IsValid(Pawn) ? Pawn->GetActorForwardVector() : FVector::ForwardVector;
	ManualSwitchSnapshot.Time = GetWorld()->GetTimeSeconds();
	ManualSwitchSnapshot.bFirstGrab = Current == nullptr;

	SetTarget(NewTarget, EYSTargetChangeReason::Manual);

	// 타겟 전환도 스위치 창을 열 수 있다. 열지 말지 · 남용 제어는 스쿼드가 설정을 보고 정한다.
	if ( const UGameInstance* GameInstance = GetWorld()->GetGameInstance() )
	{
		if ( UYSSquadSubsystem* Squad = GameInstance->GetSubsystem<UYSSquadSubsystem>() )
		{
			Squad->TryOpenSwitchWindow(EYSSwitchTrigger::TargetSwitch, NewTarget, 0.f);
		}
	}

	return true;
}

void UYSCombatTargetComponent::ClearTarget()
{
	SetTarget(nullptr, EYSTargetChangeReason::Auto);
}

EYSTargetDistanceBand UYSCombatTargetComponent::GetDistanceBand(const AActor* Target)
{
	const APawn* Pawn = GetOwnerPawn();
	const UYSCombatProfile* Profile = ActiveProfile.Get();

	if ( IsValid(Target) == false || IsValid(Pawn) == false || IsValid(Profile) == false )
	{
		return EYSTargetDistanceBand::OutOfRange;
	}

	const float Distance = FVector::Dist(Pawn->GetActorLocation(), Target->GetActorLocation());

	// 이력은 지금 타겟 하나만 기억한다. 다른 대상을 물으면 경계값 그대로 낸다.
	if ( Target != CurrentTarget.Get() )
	{
		return Profile->RawBand(Distance);
	}

	const EYSTargetDistanceBand Previous = bHasLastBand ? LastBand : Profile->RawBand(Distance);
	LastBand = Profile->ResolveDistanceBand(Distance, Previous);
	bHasLastBand = true;

	return LastBand;
}

void UYSCombatTargetComponent::MaintainTarget()
{
	if ( CurrentTarget.IsValid() == false )
	{
		// 타겟이 파괴돼 약참조가 끊겼다. 차선을 고르고, 없으면 "타겟 없음" 을 알린다.
		if ( bHasTarget )
		{
			TArray<AActor*> Candidates;
			GatherCandidates(Candidates);
			AActor* Next = SelectBest(Candidates);

			CurrentTarget.Reset();
			bHasTarget = false;
			bHasLastBand = false;
			LostSinceTime = -1.f;

			if ( IsValid(Next) )
			{
				SetTarget(Next, EYSTargetChangeReason::Auto);
			}
			else
			{
				OnTargetChanged.Broadcast(nullptr, nullptr, EYSTargetChangeReason::Auto);
			}
		}
		return;
	}

	if ( IsJustAvoidWindowActive() )
	{
		return;
	}

	AActor* Current = CurrentTarget.Get();

	// 사망 · 제외 상태는 유지 시간 없이 바로 차선으로 간다.
	if ( IsExcluded(Current) )
	{
		TArray<AActor*> Candidates;
		GatherCandidates(Candidates);
		SetTarget(SelectBest(Candidates), EYSTargetChangeReason::Auto);
		return;
	}

	if ( IsValidCandidate(Current) )
	{
		LostSinceTime = -1.f;
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();

	if ( LostSinceTime < 0.f )
	{
		LostSinceTime = Now;
		return;
	}

	const UYSCombatProfile* Profile = ActiveProfile.Get();
	const float Grace = IsValid(Profile) ? Profile->LoseGraceTime : 0.f;

	if ( Now - LostSinceTime >= Grace )
	{
		TArray<AActor*> Candidates;
		GatherCandidates(Candidates);
		SetTarget(SelectBest(Candidates), EYSTargetChangeReason::Auto);
	}
}

TArray<AActor*> UYSCombatTargetComponent::GetNearbyEnemies() const
{
	TArray<AActor*> Result;
	Result.Reserve(NearbyEnemies.Num());

	for ( const TWeakObjectPtr<AActor>& Enemy : NearbyEnemies )
	{
		if ( AActor* Actor = Enemy.Get() )
		{
			Result.Add(Actor);
		}
	}

	return Result;
}

void UYSCombatTargetComponent::ScanNearbyEnemies()
{
	const APawn* Pawn = GetOwnerPawn();
	const UYSCombatProfile* Profile = ActiveProfile.Get();

	// 교대 중 폰이 잠깐 비는 틈은 판정하지 않는다. 0명으로 세면 교전 종료 대기가 흐른다.
	if ( IsValid(Pawn) == false || IsValid(Profile) == false )
	{
		return;
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(YSCombatAwarenessGather), false, Pawn);
	TArray<FOverlapResult> Overlaps;

	GetWorld()->OverlapMultiByObjectType(Overlaps, Pawn->GetActorLocation(), FQuat::Identity,
		FCollisionObjectQueryParams(ECC_Pawn), FCollisionShape::MakeSphere(Profile->AttackRange), Params);

	TArray<AActor*> Found;

	for ( const FOverlapResult& Overlap : Overlaps )
	{
		AActor* Candidate = Overlap.GetActor();

		if ( IsValid(Candidate) && Found.Contains(Candidate) == false && IsAware(Candidate) )
		{
			Found.Add(Candidate);
		}
	}

	if ( HasSameNearbyEnemies(Found) == false )
	{
		NearbyEnemies.Reset(Found.Num());

		for ( AActor* Enemy : Found )
		{
			NearbyEnemies.Add(Enemy);
		}

		OnNearbyEnemiesChanged.Broadcast(Found);
	}

	UpdateEngagement(Found.Num() > 0);
}

bool UYSCombatTargetComponent::IsAware(const AActor* Candidate) const
{
	const APawn* Pawn = GetOwnerPawn();
	const UYSCombatProfile* Profile = ActiveProfile.Get();

	if ( FGenericTeamId::GetAttitude(Pawn, Candidate) != ETeamAttitude::Hostile )
	{
		return false;
	}

	const FVector TargetLocation = Candidate->GetActorLocation();

	if ( FVector::DistSquared(Pawn->GetActorLocation(), TargetLocation) > FMath::Square(Profile->AttackRange) )
	{
		return false;
	}

	if ( IsExcluded(Candidate) )
	{
		return false;
	}

	FVector ViewLocation;
	FRotator ViewRotation;

	if ( Profile->AwarenessViewAngle < 360.f && GetViewPoint(ViewLocation, ViewRotation) )
	{
		const FVector ToTarget = ( TargetLocation - ViewLocation ).GetSafeNormal();
		const float Angle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(ViewRotation.Vector(), ToTarget)));

		if ( Angle > Profile->AwarenessViewAngle * 0.5f )
		{
			return false;
		}
	}

	FCollisionQueryParams Params(SCENE_QUERY_STAT(YSCombatAwarenessSight), false, Pawn);
	Params.AddIgnoredActor(Candidate);

	FHitResult Hit;
	return GetWorld()->LineTraceSingleByChannel(Hit, Pawn->GetPawnViewLocation(), TargetLocation, ECC_Visibility, Params) == false;
}

bool UYSCombatTargetComponent::HasSameNearbyEnemies(const TArray<AActor*>& Found) const
{
	if ( Found.Num() != NearbyEnemies.Num() )
	{
		return false;
	}

	// 파괴돼 끊긴 약참조는 Found 에 없으므로 여기서 다름으로 걸린다.
	for ( const TWeakObjectPtr<AActor>& Enemy : NearbyEnemies )
	{
		if ( Enemy.IsValid() == false || Found.Contains(Enemy.Get()) == false )
		{
			return false;
		}
	}

	return true;
}

void UYSCombatTargetComponent::UpdateEngagement(bool bAnyEnemy)
{
	if ( bAnyEnemy )
	{
		EmptySinceTime = -1.f;

		if ( bEngaged == false )
		{
			BeginEngagement();
		}
		return;
	}

	if ( bEngaged == false )
	{
		return;
	}

	const float Now = GetWorld()->GetTimeSeconds();

	if ( EmptySinceTime < 0.f )
	{
		EmptySinceTime = Now;
	}

	const UYSCombatProfile* Profile = ActiveProfile.Get();
	const float Delay = IsValid(Profile) ? Profile->EngagementEndDelay : 0.f;

	if ( Now - EmptySinceTime >= Delay )
	{
		EndEngagement();
	}
}

void UYSCombatTargetComponent::BeginEngagement()
{
	bEngaged = true;
	OnEngagementBegan.Broadcast();

	TryEngageTarget();

	if ( CurrentTarget.IsValid() == false )
	{
		StartEngageRetry();
	}
}

void UYSCombatTargetComponent::EndEngagement()
{
	// 타겟을 비울 때 재시도가 다시 켜지지 않게 상태를 먼저 내린다.
	bEngaged = false;
	EmptySinceTime = -1.f;

	StopEngageRetry();
	ClearTarget();

	OnEngagementEnded.Broadcast();
}

void UYSCombatTargetComponent::HandleOwnTargetChanged(AActor* OldTarget, AActor* NewTarget, EYSTargetChangeReason Reason)
{
	if ( IsValid(NewTarget) == false && bEngaged )
	{
		StartEngageRetry();
	}
}

void UYSCombatTargetComponent::TryEngageTarget()
{
	if ( CurrentTarget.IsValid() || bEngaged == false )
	{
		StopEngageRetry();
		return;
	}

	// 저스트 파생 대상은 이 규칙이 덮어쓰지 않는다. 창이 닫히면 다음 재시도에서 잡는다.
	if ( IsJustAvoidWindowActive() )
	{
		return;
	}

	TArray<AActor*> Candidates;
	GatherCandidates(Candidates);

	if ( AActor* Best = SelectBest(Candidates) )
	{
		SetTarget(Best, EYSTargetChangeReason::Engage);
		StopEngageRetry();
	}
}

void UYSCombatTargetComponent::StartEngageRetry()
{
	UWorld* World = GetWorld();

	if ( World == nullptr || World->GetTimerManager().IsTimerActive(EngageRetryTimer) )
	{
		return;
	}

	World->GetTimerManager().SetTimer(EngageRetryTimer, this, &UYSCombatTargetComponent::TryEngageTarget, FMath::Max(MaintainInterval, 0.01f), true);
}

void UYSCombatTargetComponent::StopEngageRetry()
{
	if ( UWorld* World = GetWorld() )
	{
		World->GetTimerManager().ClearTimer(EngageRetryTimer);
	}
}
