// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GraphEditor.h"
#include "Toolkits/AssetEditorToolkit.h"
#include "UObject/GCObject.h"

class IDetailsView;
class UYSPlaybackGraphAsset;
struct FEdGraphEditAction;
struct FPropertyChangedEvent;

// FGCObject 를 같이 상속하는 이유 — 편집 중인 에셋을 강참조로 붙들어야 에디터가 열려 있는 동안 GC 에 수거되지 않는다.
class FYSPlaybackGraphEditor : public FAssetEditorToolkit, public FGCObject
{
public:
	virtual ~FYSPlaybackGraphEditor() override;

	void InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UYSPlaybackGraphAsset* InAsset);

	//~ FAssetEditorToolkit
	virtual void RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual void UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager) override;
	virtual FName GetToolkitFName() const override;
	virtual FText GetBaseToolkitName() const override;
	virtual FString GetWorldCentricTabPrefix() const override;
	virtual FLinearColor GetWorldCentricTabColorScale() const override;
	//~ End

	//~ FGCObject
	virtual void AddReferencedObjects(FReferenceCollector& Collector) override;
	virtual FString GetReferencerName() const override;
	//~ End

private:
	void EnsureGraphExists();

	TSharedRef<SGraphEditor> CreateGraphEditorWidget();

	// SGraphEditor 는 Delete 조차 스스로 처리하지 않는다.
	void CreateGraphCommands();

	void DeleteSelectedNodes();
	bool CanDeleteNodes() const;
	void SelectAllNodes();
	void BreakSelectedNodeLinks();
	bool HasSelectedNodes() const;

	// InitAssetEditor 보다 먼저 붙여야 한다.
	void ExtendToolbar();

	void OnGraphChanged(const FEdGraphEditAction& Action);

	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);

	void ImportFromSourceAbility();

	TSharedRef<SDockTab> SpawnTab_GraphCanvas(const FSpawnTabArgs& Args);
	TSharedRef<SDockTab> SpawnTab_Details(const FSpawnTabArgs& Args);

	void OnSelectedNodesChanged(const FGraphPanelSelectionSet& NewSelection);

private:
	static const FName GraphCanvasTabId;
	static const FName DetailsTabId;

	TObjectPtr<UYSPlaybackGraphAsset> EditedAsset = nullptr;

	TSharedPtr<SGraphEditor> GraphEditorWidget;
	TSharedPtr<IDetailsView> DetailsView;
	TSharedPtr<FUICommandList> GraphEditorCommands;

	FDelegateHandle GraphChangedHandle;
};
