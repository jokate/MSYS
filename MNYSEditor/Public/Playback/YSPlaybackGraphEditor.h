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

/**
 * 플레이백 그래프 에디터 툴킷.
 *
 * 탭 구성은 그래프 캔버스 + 디테일 둘뿐이다.
 * FGCObject 를 같이 상속하는 이유 — 편집 중인 에셋을 툴킷이 강참조로 붙들어야
 * 에디터가 열려 있는 동안 GC 에 수거되지 않는다.
 */
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
	/** 에셋에 편집용 그래프가 없으면 만든다. 구버전 에셋을 열 때도 여기서 살아난다. */
	void EnsureGraphExists();

	TSharedRef<SGraphEditor> CreateGraphEditorWidget();

	/** 그래프 캔버스에서 먹히는 커맨드. SGraphEditor 는 Delete 조차 스스로 처리하지 않는다. */
	void CreateGraphCommands();

	void DeleteSelectedNodes();
	bool CanDeleteNodes() const;
	void SelectAllNodes();
	void BreakSelectedNodeLinks();
	bool HasSelectedNodes() const;

	/** 툴바에 가져오기 버튼을 단다. InitAssetEditor 보다 먼저 붙여야 한다. */
	void ExtendToolbar();

	/** 그래프 구조가 바뀌었을 때. 여기서 런타임 데이터를 다시 굽는다. */
	void OnGraphChanged(const FEdGraphEditAction& Action);

	/** 노드 프로퍼티가 바뀌었을 때. 구조는 그대로여도 엣지 내용이 달라진다. */
	void OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent);

	/** 레거시 배열에서 그래프를 다시 만든다. 툴바 버튼이 부른다. */
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
