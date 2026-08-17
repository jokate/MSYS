// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "SGraphPin.h"

class SOverlay;
class SInlineEditableTextBlock;
class UYSPlaybackGraphNode_Base;

/**
 * 상태 노드의 핀.
 *
 * 노드 전체를 덮는 투명한 핀이다. 그래서 사용자는 "노드를 끌어다 노드에 놓는다"고 느끼지만
 * 실제로는 핀에서 핀으로 선을 긋고 있다. AnimBP 의 SStateMachineOutputPin 이 같은 트릭이다.
 */
class SYSPlaybackStatePin : public SGraphPin
{
public:
	SLATE_BEGIN_ARGS(SYSPlaybackStatePin) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UEdGraphPin* InPin);

protected:
	virtual TSharedRef<SWidget> GetDefaultValueWidget() override;

	const FSlateBrush* GetPinBorder() const;
};


/** 상태 노드의 외형. 둥근 본체에 이름만 크게 뜬다. 핀은 보이지 않는다. */
class SGraphNodeYSPlaybackState : public SGraphNode
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeYSPlaybackState) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UYSPlaybackGraphNode_Base* InNode);

	//~ SGraphNode
	virtual void UpdateGraphNode() override;
	virtual void CreatePinWidgets() override;
	virtual void AddPin(const TSharedRef<SGraphPin>& PinToAdd) override;
	virtual void GetNodeInfoPopups(FNodeInfoContext* Context, TArray<FGraphInformationPopupInfo>& Popups) const override {}
	//~ End

protected:
	FSlateColor GetBorderBackgroundColor() const;

	void OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo);
	bool OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage);

	/** 본체·색상 브러시. 시작 노드는 다른 색을 쓰려고 파생에서 갈아끼운다. */
	virtual const FSlateBrush* GetBodyBrush() const;
	virtual const FSlateBrush* GetSpillBrush() const;
	virtual const FSlateBrush* GetIconBrush() const;
	virtual FLinearColor GetSpillColor() const;

	TSharedPtr<SOverlay> PinOverlay;
};


/** 시작 노드. 상태와 같은 골격에 색과 아이콘만 다르다. */
class SGraphNodeYSPlaybackEntry : public SGraphNodeYSPlaybackState
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeYSPlaybackEntry) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UYSPlaybackGraphNode_Base* InNode);

protected:
	virtual const FSlateBrush* GetBodyBrush() const override;
	virtual const FSlateBrush* GetSpillBrush() const override;
	virtual const FSlateBrush* GetIconBrush() const override;
	virtual FLinearColor GetSpillColor() const override;
};


/** 종료 노드. 체인이 여기서 끝난다는 걸 붉은 색으로 말한다. */
class SGraphNodeYSPlaybackExit : public SGraphNodeYSPlaybackState
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeYSPlaybackExit) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UYSPlaybackGraphNode_Base* InNode);

protected:
	virtual const FSlateBrush* GetBodyBrush() const override;
	virtual const FSlateBrush* GetSpillBrush() const override;
	virtual const FSlateBrush* GetIconBrush() const override;
	virtual FLinearColor GetSpillColor() const override;
};


/** 유지 노드. 전환하지 않고 제자리에 머문다. */
class SGraphNodeYSPlaybackStay : public SGraphNodeYSPlaybackState
{
public:
	SLATE_BEGIN_ARGS(SGraphNodeYSPlaybackStay) {}
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, UYSPlaybackGraphNode_Base* InNode);

protected:
	virtual const FSlateBrush* GetBodyBrush() const override;
	virtual const FSlateBrush* GetSpillBrush() const override;
	virtual const FSlateBrush* GetIconBrush() const override;
	virtual FLinearColor GetSpillColor() const override;
};
