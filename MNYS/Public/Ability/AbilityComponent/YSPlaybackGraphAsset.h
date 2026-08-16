// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YSPlaybackGraphAsset.generated.h"

class UEdGraph;
class UYSAbilityPlaybackBase;
class UYSGameplayAbility;

/**
 * 플레이백 그래프 에셋.
 *
 * 이 에셋은 두 개의 표현을 들고 있다 —
 *  - 편집용 그래프(EdGraph) : 노드 위치·핀·연결. 에디터 전용이며 쿠킹되지 않는다.
 *  - 컴파일 산출물(Playbacks) : 런타임이 실제로 읽는 데이터.
 *
 * 엔진의 USoundCue 가 같은 구조다 — 런타임은 FirstNode 를 읽고,
 * 편집은 SoundCueGraph 에서 하며, 구조가 바뀔 때마다 그래프를 런타임 데이터로 굽는다.
 * (Runtime/Engine/Classes/Sound/SoundCue.h:95,114)
 *
 * 편집용 그래프의 노드가 소유한 플레이백 오브젝트는 쿠킹 때 그래프와 함께 사라진다.
 * 그래서 컴파일은 반드시 '복제'다 — 참조를 옮기는 게 아니다.
 */
UCLASS(BlueprintType, Blueprintable)
class MNYS_API UYSPlaybackGraphAsset : public UObject
{
	GENERATED_BODY()

public:
	/**
	 * 컴파일 산출물. 어빌리티가 이 배열을 그대로 쓴다.
	 *
	 * 0번은 항상 시작 노드가 가리키는 플레이백이다 — 컴파일러가 그렇게 정렬한다.
	 * 기존 어빌리티의 Playbacks 배열과 형태가 같아서 런타임 로직이 바뀌지 않는다.
	 */
	UPROPERTY(VisibleAnywhere, Instanced, Category = "YS | Compiled", meta = (DisplayName = "컴파일된 플레이백 (읽기 전용)"))
	TArray<TObjectPtr<UYSAbilityPlaybackBase>> Playbacks;

#if WITH_EDITORONLY_DATA
	/** 편집용 그래프. 에디터 모듈이 생성하고 소유한다. 런타임은 이 값을 보지 않는다. */
	UPROPERTY()
	TObjectPtr<UEdGraph> EdGraph;

	/**
	 * 마이그레이션 원본.
	 * 이 어빌리티의 레거시 Playbacks 배열을 그래프로 옮겨온다. 툴바의 가져오기 버튼이 쓴다.
	 */
	UPROPERTY(EditAnywhere, Category = "YS | Migration", meta = (DisplayName = "가져올 어빌리티"))
	TSubclassOf<UYSGameplayAbility> SourceAbility;
#endif
};
