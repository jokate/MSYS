// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "General/YSStruct.h"
#include "StructUtils/InstancedStruct.h"
#include "UObject/Object.h"
#include "YSAbilityPlayback.generated.h"

class ALevelSequenceActor;
/**
 * 
 */
UENUM(BlueprintType)
enum class EYSAbilityPlaybackType : uint8
{
	None UMETA(DisplayName = "아무것도 재생 안함"),
	Montage UMETA(DisplayName = "몽타주 재생"),
	Sequence UMETA(DisplayName = "시퀀스 재생"),
	
};

USTRUCT()
struct FYSPlaybackEdge
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, Category = "YS | Transition", meta = (DisplayName = "발화 조건"))
	EYSPlaybackResult RequiredResult = EYSPlaybackResult::Completed;
	
	// -1 = 체인 종료 (어빌리티 EndAbility)
	UPROPERTY(EditDefaultsOnly, Category = "YS | Transition", meta = (DisplayName = "다음 노드 인덱스 (-1: 종료)"))
	int32 NextNodeIndex = INDEX_NONE;
};

UCLASS(EditInlineNew, DefaultToInstanced, CollapseCategories)
class MNYS_API UYSAbilityPlaybackBase : public UObject
{
	GENERATED_BODY()
	
public : 
	void Play(const FYSPlaybackContext& Context);
	void ReleaseMotionWarp();
	void DispatchNext(EYSPlaybackResult Result);
	
protected : 
	virtual bool CheckCondition(const FYSPlaybackContext& Context);
	
	UFUNCTION()
	void OnMontagePlayed();

	UFUNCTION()
	void OnMontageInterrupted();
	
	virtual void SetupMontage(const FYSPlaybackContext& Context);
	
	virtual void SetPlayback(const FYSPlaybackContext& Context);
	
	UFUNCTION()
	void OnSequencePlayed();
	
	virtual void SetupSequence(const FYSPlaybackContext& Context);


public : 
	UPROPERTY(EditDefaultsOnly, Category = "YS | Ability Playback Type")
	EYSAbilityPlaybackType PlaybackType = EYSAbilityPlaybackType::Montage;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Sequence", meta = (DisplayName = "시퀀스 설정", 	EditCondition = "PlaybackType == EYSAbilityPlaybackType::Sequence"))
	FYSSequencePlaySettings SequenceSettings;
	
	UPROPERTY(EditDefaultsOnly, Category = "YS | Montage To Play", meta = (DisplayName = "재생할 몽타주 정보", EditCondition = "PlaybackType == EYSAbilityPlaybackType::Montage", BaseStruct = "/Script/MNYS.YSMontageSelector", ExcludeBaseStruct))
	FInstancedStruct MontageSelector;

	UPROPERTY(EditDefaultsOnly, Category = "YS | Transitions", meta = (DisplayName = "다음 플레이백 전환"))
	TArray<FYSPlaybackEdge> Transitions;

private :
	UPROPERTY()
	TObjectPtr<ALevelSequenceActor> ActiveSequenceActor = nullptr;
	
	// Play() 시점에 캡처 — 콜백에서 컨텍스트 참조용
	FYSPlaybackContext CapturedContext;
};
