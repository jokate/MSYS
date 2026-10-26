// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbilityTypes.h"
#include "Ability/AbilityComponent/YSPlaybackAction.h"
#include "General/YSStruct.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "YSPlaybackTask.generated.h"

class UAbilityTask;
class UYSGameplayAbility;
struct FYSPlaybackContext;
class UYSAbilityPlaybackBase;

UENUM()
enum class EYSRouteTarget : uint8
{
	Node UMETA(DisplayName = "다음 노드"),
	End  UMETA(DisplayName = "체인 종료"),
	Stay UMETA(DisplayName = "유지 (전환 없음)"),
};

/** Task 출력 하나에서 갈 수 있는 목적지 후보. 그래프의 화살표 하나에 대응한다. */
USTRUCT()
struct FYSPlaybackRoute
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "YS | Route", meta = (DisplayName = "전환 조건", BaseStruct = "/Script/MNYS.YSPlaybackCondition", ExcludeBaseStruct))
	TArray<FInstancedStruct> Conditions;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Route", meta = (DisplayName = "목적지"))
	EYSRouteTarget Target = EYSRouteTarget::End;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Route", meta = (DisplayName = "다음 노드 인덱스", EditCondition = "Target == EYSRouteTarget::Node", EditConditionHides))
	int32 NextNodeIndex = INDEX_NONE;

	/** 경로가 확정되면 전환 직전에 실행한다. 어빌리티가 아직 살아 있는 시점이다. */
	UPROPERTY(EditDefaultsOnly, Instanced, Category = "YS | Route", meta = (DisplayName = "경로 행동"))
	TArray<TObjectPtr<UYSPlaybackAction>> Actions;
};

USTRUCT()
struct FYSPlaybackOutputRoutes
{
	GENERATED_BODY()

	// 배열 순서가 우선순위. 조건을 통과한 첫 경로로 간다.
	UPROPERTY(EditDefaultsOnly, Category = "YS | Route", meta = (DisplayName = "경로"))
	TArray<FYSPlaybackRoute> Routes;
};

/**
 * 플레이백 자체에서 처리될 일들을 의미함.
 * 순서에 의거해서 동작하는 방식으로 진행한다.
 */
UCLASS(Abstract, EditInlineNew, DefaultToInstanced, CollapseCategories)
class MNYS_API UYSPlaybackTask : public UObject
{
	GENERATED_BODY()
	
public :
	void Start(UYSAbilityPlaybackBase* InOwnerNode);
	void Stop();
	bool IsRunning() const { return bRunning; }

	/** 마지막 Start 에서 실제로 일이 시작됐는가. 몽타주가 없거나 재생에 실패한 Task 는 false. */
	bool DidStartWork() const { return bStartedWork; }

	virtual bool HandleInput(const FGameplayTag& InInputTag, EYSInputPhase InInputPhase) { return false; }
	virtual bool HandleInputWindowClosed() { return false; }
	virtual void HandleHit(const TArray<FHitResult>& HitResults) {}
	virtual void HandleContextTagChanged() {}
	virtual void ReleaseMotionWarp() {}

	virtual int32 GetOutputCount() const { return 1; }
	virtual FText GetOutputDisplayName(int32 OutputIndex) const;

	/** 출력이 노드 수명 동안 여러 번 날 수 있는가. 아니면 유지(Stay) 경로가 노드를 멈춘 채 남긴다. */
	virtual bool IsRepeatable() const { return false; }

	void ResetRoutes();
	void AddRoute(int32 OutputIndex, const FYSPlaybackRoute& Route);

protected :
	virtual void OnStart() {}
	virtual void OnStop() {}

	/** 이 출력의 경로를 우선순위대로 평가한다. 통과한 경로가 없으면 전환하지 않고 false. */
	bool TryResolve(int32 OutputIndex = 0);

	/** TryResolve 가 실패하면 체인을 끝낸다. 재생 완료처럼 되돌아갈 곳이 없는 출력에 쓴다. */
	void Resolve(int32 OutputIndex = 0);

	template <typename TaskType>
	TaskType* Track(TaskType* Task)
	{
		if ( IsValid(Task) )
		{
			RunningTasks.Add(Task);
		}
		return Task;
	}

	UYSGameplayAbility* GetOwningAbility() const;
	TSharedPtr<FYSPlaybackContext> GetContext() const;

	/** OnStart 가 아무 일도 시작하지 못했을 때 부른다. 기본은 일이 시작된 것으로 본다. */
	void MarkWorkNotStarted() { bStartedWork = false; }

public :
	// 인덱스 = 출력. 그래프 에셋에서는 컴파일러가 화살표로 덮어쓴다.
	UPROPERTY(EditDefaultsOnly, Category = "YS | Transition", meta = (DisplayName = "출력별 경로"))
	TArray<FYSPlaybackOutputRoutes> OutputRoutes;

private :
	UPROPERTY(Transient)
	TObjectPtr<UYSAbilityPlaybackBase> OwnerNode = nullptr;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UAbilityTask>> RunningTasks;

	bool bRunning = false;
	bool bStartedWork = true;
};


UCLASS(DisplayName = "몽타주 재생")
class MNYS_API UYSPlaybackTask_PlayMontage : public UYSPlaybackTask
{
	GENERATED_BODY()
	
public :
	static constexpr int32 Output_Completed = 0;
	static constexpr int32 Output_Interrupted = 1;

	virtual void ReleaseMotionWarp() override;
	virtual int32 GetOutputCount() const override { return 2; }
	virtual FText GetOutputDisplayName(int32 OutputIndex) const override;

protected :
	virtual void OnStart() override;
	virtual void OnStop() override;

private :
	UFUNCTION()
	void OnMontageFinished();

	UFUNCTION()
	void OnMontageInterrupted();

public :
	UPROPERTY(EditDefaultsOnly, Category = "YS | Montage To Play", meta = (DisplayName = "재생할 몽타주 정보", BaseStruct = "/Script/MNYS.YSMontageSelector", ExcludeBaseStruct))
	FInstancedStruct MontageSelector;
};


UCLASS(DisplayName = "시퀀스 재생")
class MNYS_API UYSPlaybackTask_PlaySequence : public UYSPlaybackTask
{
	GENERATED_BODY()
	
public :
	virtual void OnStart() override;
	
	
private : 
	UFUNCTION()
	void OnSequenceFinished();
	
public : 
	UPROPERTY(EditDefaultsOnly, Category = "YS | Transition", meta = (DisplayName = "시퀀스 재생 설정"))
	FYSSequencePlaySettings SequenceSettings;
};

UCLASS(DisplayName = "입력 대기")
class MNYS_API UYSPlaybackTask_WaitInput : public UYSPlaybackTask
{
	GENERATED_BODY()

public :
	virtual bool HandleInput(const FGameplayTag& InInputTag, EYSInputPhase InInputPhase) override;
	virtual bool HandleInputWindowClosed() override;
	virtual FText GetOutputDisplayName(int32 OutputIndex) const override;
	virtual bool IsRepeatable() const override { return true; }

protected :
	virtual void OnStart() override;

public :
	// 비워두면 입력을 받지 않고, 입력 창이 닫힐 때 경로 조건만 본다 (InputHeld 로 도는 연사 루프).
	UPROPERTY(EditDefaultsOnly, Category = "YS | Input", meta = (DisplayName = "대기할 입력"))
	FGameplayTag InputTag;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Input", meta = (DisplayName = "입력 단계"))
	EYSInputPhase InputPhase = EYSInputPhase::Pressed;

	// 끄면 입력 즉시 전환 (조준 취소, 차지 릴리즈). 켜면 입력 창이 닫힐 때 전환 (콤보, 연사).
	UPROPERTY(EditDefaultsOnly, Category = "YS | Input", meta = (DisplayName = "입력 창이 닫힐 때 전환"))
	bool bWaitInputWindowClose = false;

private :
	bool bInputReceived = false;
};

UCLASS(DisplayName = "이벤트 대기")
class MNYS_API UYSPlaybackTask_WaitEvent : public UYSPlaybackTask
{
	GENERATED_BODY()
	
public :
	virtual void OnStart() override;
	virtual FText GetOutputDisplayName(int32 OutputIndex) const override;
	virtual bool IsRepeatable() const override { return true; }

	UFUNCTION()
	void OnEventReceived(FGameplayEventData Payload);

public:
	UPROPERTY(EditDefaultsOnly, Category = "YS | Input", meta = (DisplayName = "대기할 이벤트"))
	FGameplayTag EventTag;
};


UCLASS(DisplayName = "N초 대기")
class MNYS_API UYSPlaybackTask_WaitDelay : public UYSPlaybackTask
{
	GENERATED_BODY()
	
public :
	virtual void OnStart() override;
	
	UFUNCTION()
	void OnDelayFinished();
	
public : 
	UPROPERTY(EditDefaultsOnly, Category = "YS | Delay", meta = (DisplayName = "대기 시간"))
	float DelayTime = 0.f;
};
/**
 * 스위치 어택 진입기 형 고르기. 스위치 창 안 입력(없으면 창 닫힘 · 창 상한 시간)으로 진입기 형 표의 행을 고르고,
 * 그 행의 판정 지연 뒤 판정한다. 형마다 다른 것은 표(FYSSwitchEntryForm)에만 있다 — 형이 늘면 표에 행을 더한다.
 * 출력은 판정 직후 한 번. 경로가 없으면 전환하지 않고 몽타주가 노드를 끝낸다.
 */
UCLASS(DisplayName = "진입기 형 고르기")
class MNYS_API UYSPlaybackTask_SwitchEntry : public UYSPlaybackTask
{
	GENERATED_BODY()

public :
	virtual bool HandleInput(const FGameplayTag& InInputTag, EYSInputPhase InInputPhase) override;
	virtual bool HandleInputWindowClosed() override;
	virtual void ReleaseMotionWarp() override;
	virtual FText GetOutputDisplayName(int32 OutputIndex) const override;

protected :
	virtual void OnStart() override;
	virtual void OnStop() override;

private :
	UFUNCTION()
	void OnWindowTimedOut();

	UFUNCTION()
	void OnHitDelayFinished();

	void SelectForm(FName RowName);
	void ExecuteHit();

	AActor* FindCombatTarget(const AActor* Instigator) const;

public :
	// 입력 창이 닫히는 신호가 오지 않아도 이 시간이 지나면 무입력 형으로 나간다. 스위치 창 GE 길이와 맞춘다.
	UPROPERTY(EditDefaultsOnly, Category = "YS | SwitchEntry", meta = (DisplayName = "입력 창 상한 (초)", ClampMin = "0"))
	float InputWindowLimit = 0.5f;

private :
	FName SelectedForm = NAME_None;
	bool bSelected = false;
	bool bWarpSet = false;
};

/**
 * 컨텍스트 스위치 변형의 이동과 판정. UYSGameplayAbility_ContextSwitch 가 고른 변형 표 행을 읽는다.
 * 시작할 때 이동(회전 · 워프)을 걸고, 행의 판정 지연 뒤 한 번 친다. 행마다 다른 것은 표에만 있다.
 * 등급 0 · 1 적은 그로기 피해 대신 경직 GE, 피해 · 그로기 피해 · 경직 길이에는 남용 감쇠 배율을 곱한다.
 * 출력은 판정 직후 한 번. 경로가 없으면 전환하지 않고 몽타주가 노드를 끝낸다.
 */
UCLASS(DisplayName = "컨텍스트 스위치 변형 판정")
class MNYS_API UYSPlaybackTask_ContextSwitchHit : public UYSPlaybackTask
{
	GENERATED_BODY()

public :
	virtual void ReleaseMotionWarp() override;
	virtual FText GetOutputDisplayName(int32 OutputIndex) const override;

protected :
	virtual void OnStart() override;
	virtual void OnStop() override;

private :
	UFUNCTION()
	void OnHitDelayFinished();

	void ExecuteHit();

	const struct FYSContextSwitchVariant* GetVariant() const;

private :
	bool bWarpSet = false;
};
