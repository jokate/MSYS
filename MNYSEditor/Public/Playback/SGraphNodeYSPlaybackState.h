// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGraphNode.h"
#include "SGraphPin.h"

class SOverlay;
class SInlineEditableTextBlock;
class UYSPlaybackGraphNode_Base;

// 노드 전체를 덮는 투명한 핀 — 노드를 끌어다 놓는 것처럼 느껴지지만 실제로는 핀에서 핀으로 선을 긋는다.
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

	// 시작·종료·유지 노드가 파생에서 갈아끼운다.
	virtual const FSlateBrush* GetBodyBrush() const;
	virtual const FSlateBrush* GetSpillBrush() const;
	virtual const FSlateBrush* GetIconBrush() const;
	virtual FLinearColor GetSpillColor() const;

	TSharedPtr<SOverlay> PinOverlay;
};


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
