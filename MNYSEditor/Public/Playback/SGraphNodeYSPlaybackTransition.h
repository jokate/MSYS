// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UYSPlaybackGraphNode_Transition;

/**
 * 전환 노드의 외형.
 *
 * 이 위젯은 자기 위치를 스스로 정하지 않는다 —
 * 두 상태 노드의 최종 위치가 정해진 뒤(2차 배치)에야 그 사이에 끼어든다.
 * 그래서 화면에서는 화살표 위에 얹힌 라벨처럼 보인다.
 * 엔진의 SGraphNodeAnimTransition 이 같은 방식이다.
 */
class SGraphNodeYSPlaybackTransition : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeYSPlaybackTransition) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UYSPlaybackGraphNode_Transition* InNode);

	//~ SGraphNode
	virtual void UpdateGraphNode() override;

	/** 핀은 그리지 않는다. 이 노드는 선 위의 라벨이지 상자가 아니다. */
	virtual void CreatePinWidgets() override {}
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override {}

	/** 위치는 연결된 상태들이 정한다. 드래그로 옮길 수 없다. */
	virtual void MoveTo(const FVector2f& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty = true) override {}

	/**
	 * 상태 노드보다 뒤에 정렬시킨다.
	 *
	 * SNodePanel 은 이 값 오름차순으로 자식을 배치하고, 히트 테스트는 그 역순으로 훑는다.
	 * 기본값(0)이면 전환이 상태보다 먼저 배치돼 겹치는 순간 클릭을 상태가 먹는다.
	 * 자기 자신으로 도는 전환은 늘 상태 위에 얹히므로 항상 그 상황이 된다.
	 */
	virtual int32 GetSortDepth() const override { return 1; }

	virtual bool RequiresSecondPassLayout() const override { return true; }
	virtual void PerformSecondPassLayout(const TMap<UObject*, TSharedRef<SNode>>& NodeToWidgetLookup) const override;

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	                      FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	//~ End

private:
	/** 두 상태 사이 선분 위로 자기 좌표를 옮긴다. 같은 쌍에 전환이 여럿이면 서로 어긋나게 둔다. */
	void PositionBetweenTwoNodes(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const;

	FText GetTransitionTitle() const;

	/** 조건이 붙은 전환과 그냥 흘러가는 전환을 색으로 가른다. */
	FSlateColor GetTransitionColor() const;
};
