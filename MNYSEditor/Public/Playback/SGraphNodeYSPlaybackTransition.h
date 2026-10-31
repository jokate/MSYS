// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"

class UYSPlaybackGraphNode_Transition;

// 두 상태 노드의 최종 위치가 정해진 뒤(2차 배치)에야 그 사이에 끼어들어, 화살표 위에 얹힌 라벨처럼 보인다.
class SGraphNodeYSPlaybackTransition : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeYSPlaybackTransition) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UYSPlaybackGraphNode_Transition* InNode);

	//~ SGraphNode
	virtual void UpdateGraphNode() override;

	// 핀은 그리지 않고, 위치는 연결된 상태들이 정한다(드래그 불가).
	virtual void CreatePinWidgets() override {}
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override {}

	virtual void MoveTo(const FVector2f& NewPosition, FNodeSet& NodeFilter, bool bMarkDirty = true) override {}

	// SNodePanel 은 이 값 오름차순으로 배치하고 히트 테스트는 역순으로 훑는다.
	// 기본값(0)이면 전환이 상태보다 먼저 배치돼 겹칠 때 클릭을 상태가 먹는다 — 자기 루프는 늘 그렇다.
	virtual int32 GetSortDepth() const override { return 1; }

	virtual bool RequiresSecondPassLayout() const override { return true; }
	virtual void PerformSecondPassLayout(const TMap<UObject*, TSharedRef<SNode>>& NodeToWidgetLookup) const override;

	virtual int32 OnPaint(const FPaintArgs& Args, const FGeometry& AllottedGeometry, const FSlateRect& MyCullingRect,
	                      FSlateWindowElementList& OutDrawElements, int32 LayerId, const FWidgetStyle& InWidgetStyle, bool bParentEnabled) const override;
	//~ End

private:
	void PositionBetweenTwoNodes(const FGeometry& StartGeom, const FGeometry& EndGeom, int32 NodeIndex, int32 MaxNodes) const;

	FText GetTransitionTitle() const;

	FSlateColor GetTransitionColor() const;
};
