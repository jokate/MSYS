// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/YSPlaybackGraphEditor.h"

#include "Ability/AbilityComponent/YSPlaybackGraphAsset.h"
#include "Framework/Commands/GenericCommands.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "GraphEditorActions.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Modules/ModuleManager.h"
#include "Playback/YSPlaybackGraph.h"
#include "Playback/YSPlaybackGraphCompiler.h"
#include "Playback/YSPlaybackGraphMigration.h"
#include "Playback/YSPlaybackGraphNode.h"
#include "Playback/YSPlaybackGraphSchema.h"
#include "PropertyEditorModule.h"
#include "ScopedTransaction.h"
#include "Styling/AppStyle.h"
#include "Widgets/Docking/SDockTab.h"

#define LOCTEXT_NAMESPACE "YSPlaybackGraphEditor"

const FName FYSPlaybackGraphEditor::GraphCanvasTabId(TEXT("YSPlaybackGraphEditor_GraphCanvas"));
const FName FYSPlaybackGraphEditor::DetailsTabId(TEXT("YSPlaybackGraphEditor_Details"));

FYSPlaybackGraphEditor::~FYSPlaybackGraphEditor()
{
	// 그래프가 툴킷보다 오래 살아서, 핸들을 안 놓으면 열고 닫을 때마다 리스너가 쌓인다.
	if (GraphChangedHandle.IsValid() && IsValid(EditedAsset) && EditedAsset->EdGraph != nullptr)
	{
		EditedAsset->EdGraph->RemoveOnGraphChangedHandler(GraphChangedHandle);
	}
}

void FYSPlaybackGraphEditor::InitEditor(const EToolkitMode::Type Mode, const TSharedPtr<IToolkitHost>& InitToolkitHost, UYSPlaybackGraphAsset* InAsset)
{
	EditedAsset = InAsset;

	EnsureGraphExists();

	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>(TEXT("PropertyEditor"));

	FDetailsViewArgs DetailsViewArgs;
	DetailsViewArgs.bHideSelectionTip = true;
	DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::HideNameArea;

	// 프로퍼티가 전부 EditDefaultsOnly 라서, 기본값(Automatic)이면 에셋 안에서는 통째로 사라진다.
	DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Show;

	DetailsView = PropertyModule.CreateDetailView(DetailsViewArgs);
	DetailsView->SetObject(EditedAsset);

	DetailsView->OnFinishedChangingProperties().AddSP(this, &FYSPlaybackGraphEditor::OnFinishedChangingProperties);

	GraphEditorWidget = CreateGraphEditorWidget();

	// 저장을 기다리면 PIE 가 옛 데이터로 돌아서 구조가 바뀌면 즉시 다시 굽는다.
	GraphChangedHandle = EditedAsset->EdGraph->AddOnGraphChangedHandler(
		FOnGraphChanged::FDelegate::CreateSP(this, &FYSPlaybackGraphEditor::OnGraphChanged));

	FYSPlaybackGraphCompiler::Compile(EditedAsset);

	ExtendToolbar();

	const TSharedRef<FTabManager::FLayout> Layout = FTabManager::NewLayout(TEXT("Standalone_YSPlaybackGraphEditor_v1"))
		->AddArea
		(
			FTabManager::NewPrimaryArea()->SetOrientation(Orient_Vertical)
			->Split
			(
				FTabManager::NewSplitter()->SetOrientation(Orient_Horizontal)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.75f)
					->AddTab(GraphCanvasTabId, ETabState::OpenedTab)
				)
				->Split
				(
					FTabManager::NewStack()
					->SetSizeCoefficient(0.25f)
					->AddTab(DetailsTabId, ETabState::OpenedTab)
				)
			)
		);

	constexpr bool bCreateDefaultStandaloneMenu = true;
	constexpr bool bCreateDefaultToolbar = true;

	InitAssetEditor(Mode, InitToolkitHost, FName(TEXT("YSPlaybackGraphEditorApp")), Layout,
		bCreateDefaultStandaloneMenu, bCreateDefaultToolbar, EditedAsset);
}

void FYSPlaybackGraphEditor::EnsureGraphExists()
{
	if (IsValid(EditedAsset) == false)
	{
		return;
	}

	if (EditedAsset->EdGraph != nullptr)
	{
		return;
	}

	EditedAsset->EdGraph = FBlueprintEditorUtils::CreateNewGraph(
		EditedAsset,
		NAME_None,
		UYSPlaybackGraph::StaticClass(),
		UYSPlaybackGraphSchema::StaticClass());

	EditedAsset->EdGraph->bAllowDeletion = false;

	// CreateNewGraph 는 기본 노드를 만들어주지 않아 시작 노드를 직접 찍어야 한다.
	EditedAsset->EdGraph->GetSchema()->CreateDefaultNodesForGraph(*EditedAsset->EdGraph);
}

TSharedRef<SGraphEditor> FYSPlaybackGraphEditor::CreateGraphEditorWidget()
{
	CreateGraphCommands();

	FGraphAppearanceInfo AppearanceInfo;
	AppearanceInfo.CornerText = LOCTEXT("AppearanceCornerText", "PLAYBACK");

	SGraphEditor::FGraphEditorEvents GraphEvents;
	GraphEvents.OnSelectionChanged = SGraphEditor::FOnSelectionChanged::CreateSP(this, &FYSPlaybackGraphEditor::OnSelectedNodesChanged);

	return SNew(SGraphEditor)
		.AdditionalCommands(GraphEditorCommands)
		.IsEditable(true)
		.Appearance(AppearanceInfo)
		.GraphToEdit(EditedAsset->EdGraph)
		.GraphEvents(GraphEvents);
}

void FYSPlaybackGraphEditor::CreateGraphCommands()
{
	if (GraphEditorCommands.IsValid())
	{
		return;
	}

	GraphEditorCommands = MakeShared<FUICommandList>();

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().Delete,
		FExecuteAction::CreateSP(this, &FYSPlaybackGraphEditor::DeleteSelectedNodes),
		FCanExecuteAction::CreateSP(this, &FYSPlaybackGraphEditor::CanDeleteNodes));

	GraphEditorCommands->MapAction(
		FGenericCommands::Get().SelectAll,
		FExecuteAction::CreateSP(this, &FYSPlaybackGraphEditor::SelectAllNodes),
		FCanExecuteAction());

	GraphEditorCommands->MapAction(
		FGraphEditorCommands::Get().BreakNodeLinks,
		FExecuteAction::CreateSP(this, &FYSPlaybackGraphEditor::BreakSelectedNodeLinks),
		FCanExecuteAction::CreateSP(this, &FYSPlaybackGraphEditor::HasSelectedNodes));
}

bool FYSPlaybackGraphEditor::HasSelectedNodes() const
{
	return GraphEditorWidget.IsValid() && GraphEditorWidget->GetSelectedNodes().Num() > 0;
}

void FYSPlaybackGraphEditor::BreakSelectedNodeLinks()
{
	if (GraphEditorWidget.IsValid() == false || EditedAsset->EdGraph == nullptr)
	{
		return;
	}

	const UEdGraphSchema* Schema = EditedAsset->EdGraph->GetSchema();

	if (Schema == nullptr)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("BreakNodeLinks", "노드 연결 끊기"));

	for (UObject* Selected : GraphEditorWidget->GetSelectedNodes())
	{
		if (UEdGraphNode* Node = Cast<UEdGraphNode>(Selected))
		{
			Schema->BreakNodeLinks(*Node);
		}
	}

	FYSPlaybackGraphCompiler::Compile(EditedAsset);
}

bool FYSPlaybackGraphEditor::CanDeleteNodes() const
{
	if (GraphEditorWidget.IsValid() == false)
	{
		return false;
	}

	for (UObject* Selected : GraphEditorWidget->GetSelectedNodes())
	{
		const UEdGraphNode* Node = Cast<UEdGraphNode>(Selected);

		if (Node != nullptr && Node->CanUserDeleteNode())
		{
			return true;
		}
	}

	return false;
}

void FYSPlaybackGraphEditor::DeleteSelectedNodes()
{
	if (GraphEditorWidget.IsValid() == false || EditedAsset->EdGraph == nullptr)
	{
		return;
	}

	const FScopedTransaction Transaction(LOCTEXT("DeleteNodes", "플레이백 노드 삭제"));

	EditedAsset->EdGraph->Modify();

	TSet<UEdGraphNode*> NodesToRemove;

	for (UObject* Selected : GraphEditorWidget->GetSelectedNodes())
	{
		UEdGraphNode* Node = Cast<UEdGraphNode>(Selected);

		if (Node == nullptr || Node->CanUserDeleteNode() == false)
		{
			continue;
		}

		NodesToRemove.Add(Node);

		// 상태에 붙어 있던 전환은 남기면 그래프에 떠다니고 컴파일에서 조용히 무시되므로 같이 지운다.
		const UYSPlaybackGraphNode_State* StateNode = Cast<UYSPlaybackGraphNode_State>(Node);

		if (StateNode == nullptr)
		{
			continue;
		}

		for (const UEdGraphPin* Pin : StateNode->Pins)
		{
			if (Pin == nullptr)
			{
				continue;
			}

			for (const UEdGraphPin* LinkedPin : Pin->LinkedTo)
			{
				if (LinkedPin == nullptr)
				{
					continue;
				}

				if (UYSPlaybackGraphNode_Transition* Transition = Cast<UYSPlaybackGraphNode_Transition>(LinkedPin->GetOwningNode()))
				{
					NodesToRemove.Add(Transition);
				}
			}
		}
	}

	if (NodesToRemove.Num() == 0)
	{
		return;
	}

	GraphEditorWidget->ClearSelectionSet();

	for (UEdGraphNode* Node : NodesToRemove)
	{
		Node->Modify();
		Node->DestroyNode();
	}

	// 파괴된 노드를 계속 보고 있으면 안 되므로 디테일을 에셋으로 되돌린다.
	if (DetailsView.IsValid())
	{
		DetailsView->SetObject(EditedAsset);
	}

	FYSPlaybackGraphCompiler::Compile(EditedAsset);
}

void FYSPlaybackGraphEditor::SelectAllNodes()
{
	if (GraphEditorWidget.IsValid())
	{
		GraphEditorWidget->SelectAllNodes();
	}
}

void FYSPlaybackGraphEditor::ExtendToolbar()
{
	const TSharedPtr<FExtender> ToolbarExtender = MakeShared<FExtender>();

	ToolbarExtender->AddToolBarExtension(
		TEXT("Asset"),
		EExtensionHook::After,
		GetToolkitCommands(),
		FToolBarExtensionDelegate::CreateLambda([this](FToolBarBuilder& Builder)
		{
			Builder.BeginSection(TEXT("YSPlayback"));

			Builder.AddToolBarButton(
				FUIAction(FExecuteAction::CreateSP(this, &FYSPlaybackGraphEditor::ImportFromSourceAbility)),
				NAME_None,
				LOCTEXT("ImportLabel", "어빌리티에서 가져오기"),
				LOCTEXT("ImportTooltip", "가져올 어빌리티에 지정한 어빌리티의 레거시 플레이백 배열을 그래프로 옮긴다. 기존 그래프는 지워진다."),
				FSlateIcon(FAppStyle::GetAppStyleSetName(), TEXT("Icons.Import")));

			Builder.EndSection();
		}));

	AddToolbarExtender(ToolbarExtender);
}

void FYSPlaybackGraphEditor::ImportFromSourceAbility()
{
	const int32 ImportedCount = FYSPlaybackGraphMigration::ImportFromSourceAbility(EditedAsset);

	if (ImportedCount <= 0)
	{
		return;
	}

	if (DetailsView.IsValid())
	{
		DetailsView->SetObject(EditedAsset);
	}

	if (GraphEditorWidget.IsValid())
	{
		GraphEditorWidget->ClearSelectionSet();
		GraphEditorWidget->ZoomToFit(false);
	}
}

void FYSPlaybackGraphEditor::OnGraphChanged(const FEdGraphEditAction& Action)
{
	FYSPlaybackGraphCompiler::Compile(EditedAsset);
}

void FYSPlaybackGraphEditor::OnFinishedChangingProperties(const FPropertyChangedEvent& PropertyChangedEvent)
{
	FYSPlaybackGraphCompiler::Compile(EditedAsset);
}

void FYSPlaybackGraphEditor::RegisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	WorkspaceMenuCategory = InTabManager->AddLocalWorkspaceMenuCategory(LOCTEXT("WorkspaceMenu_PlaybackGraph", "플레이백 그래프"));

	FAssetEditorToolkit::RegisterTabSpawners(InTabManager);

	InTabManager->RegisterTabSpawner(GraphCanvasTabId, FOnSpawnTab::CreateSP(this, &FYSPlaybackGraphEditor::SpawnTab_GraphCanvas))
		.SetDisplayName(LOCTEXT("GraphCanvasTab", "그래프"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());

	InTabManager->RegisterTabSpawner(DetailsTabId, FOnSpawnTab::CreateSP(this, &FYSPlaybackGraphEditor::SpawnTab_Details))
		.SetDisplayName(LOCTEXT("DetailsTab", "디테일"))
		.SetGroup(WorkspaceMenuCategory.ToSharedRef());
}

void FYSPlaybackGraphEditor::UnregisterTabSpawners(const TSharedRef<FTabManager>& InTabManager)
{
	FAssetEditorToolkit::UnregisterTabSpawners(InTabManager);

	InTabManager->UnregisterTabSpawner(GraphCanvasTabId);
	InTabManager->UnregisterTabSpawner(DetailsTabId);
}

TSharedRef<SDockTab> FYSPlaybackGraphEditor::SpawnTab_GraphCanvas(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("GraphCanvasTitle", "그래프"))
		[
			GraphEditorWidget.ToSharedRef()
		];
}

TSharedRef<SDockTab> FYSPlaybackGraphEditor::SpawnTab_Details(const FSpawnTabArgs& Args)
{
	return SNew(SDockTab)
		.Label(LOCTEXT("DetailsTitle", "디테일"))
		[
			DetailsView.ToSharedRef()
		];
}

void FYSPlaybackGraphEditor::OnSelectedNodesChanged(const FGraphPanelSelectionSet& NewSelection)
{
	if (DetailsView.IsValid() == false)
	{
		return;
	}

	if (NewSelection.Num() == 0)
	{
		DetailsView->SetObject(EditedAsset);
		return;
	}

	TArray<UObject*> SelectedObjects;
	SelectedObjects.Reserve(NewSelection.Num());

	for (UObject* Selected : NewSelection)
	{
		SelectedObjects.Add(Selected);
	}

	DetailsView->SetObjects(SelectedObjects);
}

void FYSPlaybackGraphEditor::AddReferencedObjects(FReferenceCollector& Collector)
{
	Collector.AddReferencedObject(EditedAsset);
}

FString FYSPlaybackGraphEditor::GetReferencerName() const
{
	return TEXT("FYSPlaybackGraphEditor");
}

FName FYSPlaybackGraphEditor::GetToolkitFName() const
{
	return FName(TEXT("YSPlaybackGraphEditor"));
}

FText FYSPlaybackGraphEditor::GetBaseToolkitName() const
{
	return LOCTEXT("AppLabel", "플레이백 그래프 에디터");
}

FString FYSPlaybackGraphEditor::GetWorldCentricTabPrefix() const
{
	return LOCTEXT("WorldCentricTabPrefix", "Playback ").ToString();
}

FLinearColor FYSPlaybackGraphEditor::GetWorldCentricTabColorScale() const
{
	return FLinearColor(0.3f, 0.2f, 0.5f, 0.5f);
}

#undef LOCTEXT_NAMESPACE
