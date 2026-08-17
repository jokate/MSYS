// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "YSPlaybackGraphAsset.generated.h"

class UEdGraph;
class UYSAbilityPlaybackBase;
class UYSGameplayAbility;

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

	/**
	 * 컴파일할 때마다 오르는 번호.
	 *
	 * 어빌리티는 이 산출물을 자기 인스턴스로 복제해서 쓴다(재생 상태가 오브젝트에 남기 때문).
	 * 이 번호가 없으면 에디터에서 그래프를 고쳐도 이미 사본을 뜬 어빌리티는 낡은 값을 계속 쓴다.
	 */
	UPROPERTY(VisibleAnywhere, Category = "YS | Compiled", meta = (DisplayName = "컴파일 일련번호"))
	int32 CompileSerial = 0;

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
