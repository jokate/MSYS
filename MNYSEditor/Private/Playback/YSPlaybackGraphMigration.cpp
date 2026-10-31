// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/YSPlaybackGraphMigration.h"

#include "Ability/YSGameplayAbility.h"
#include "Ability/AbilityComponent/YSAbilityPlayback.h"
#include "Ability/AbilityComponent/YSPlaybackCondition.h"
#include "Ability/AbilityComponent/YSPlaybackGraphAsset.h"
#include "Ability/AbilityComponent/YSPlaybackTask.h"
#include "Ability/MontageSelector/YSMontageSelector.h"
#include "Ability/MotionWarp/YSMotionWarpParam.h"
#include "Animation/AnimMontage.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "Dom/JsonObject.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "EdGraph/EdGraph.h"
#include "Engine/Blueprint.h"
#include "FileHelpers.h"
#include "HAL/IConsoleManager.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/PackageName.h"
#include "Playback/YSPlaybackGraph.h"
#include "Playback/YSPlaybackGraphCompiler.h"
#include "Playback/YSPlaybackGraphNode.h"
#include "Playback/YSPlaybackGraphSchema.h"

namespace YSPlaybackMigration
{
	static constexpr int32 StateSpacingX = 420;
	static constexpr int32 StateRowY = 0;

	static void ClearGraphExceptEntry(UEdGraph& Graph)
	{
		TArray<UEdGraphNode*> NodesToRemove;

		for (UEdGraphNode* Node : Graph.Nodes)
		{
			if (Node != nullptr && Node->IsA<UYSPlaybackGraphNode_Entry>() == false)
			{
				NodesToRemove.Add(Node);
			}
		}

		for (UEdGraphNode* Node : NodesToRemove)
		{
			Graph.RemoveNode(Node);
		}
	}

	static UYSPlaybackGraphNode_Entry* FindEntry(UEdGraph& Graph)
	{
		for (UEdGraphNode* Node : Graph.Nodes)
		{
			if (UYSPlaybackGraphNode_Entry* EntryNode = Cast<UYSPlaybackGraphNode_Entry>(Node))
			{
				return EntryNode;
			}
		}

		return nullptr;
	}
}

int32 FYSPlaybackGraphMigration::ImportFromSourceAbility(UYSPlaybackGraphAsset* Asset)
{
	if (IsValid(Asset) == false || Asset->EdGraph == nullptr)
	{
		return 0;
	}

	if (Asset->SourceAbility == nullptr)
	{
		return 0;
	}

	const UYSGameplayAbility* AbilityCDO = Asset->SourceAbility->GetDefaultObject<UYSGameplayAbility>();

	if (AbilityCDO == nullptr)
	{
		return 0;
	}

	return ImportPlaybacks(Asset, AbilityCDO->GetLegacyPlaybacks());
}

int32 FYSPlaybackGraphMigration::ImportPlaybacks(UYSPlaybackGraphAsset* Asset, const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& SourcePlaybacks, const TArray<FString>& StateNames)
{
	if (IsValid(Asset) == false || Asset->EdGraph == nullptr || SourcePlaybacks.Num() == 0)
	{
		return 0;
	}

	UEdGraph& Graph = *Asset->EdGraph;

	Graph.Modify();
	YSPlaybackMigration::ClearGraphExceptEntry(Graph);

	// 인덱스가 곧 배열 위치이므로 전환을 잇기 전에 상태 노드를 전부 만들어 둔다.
	TArray<UYSPlaybackGraphNode_State*> StateNodes;
	StateNodes.Reserve(SourcePlaybacks.Num());

	for (int32 Index = 0; Index < SourcePlaybacks.Num(); ++Index)
	{
		FGraphNodeCreator<UYSPlaybackGraphNode_State> NodeCreator(Graph);

		UYSPlaybackGraphNode_State* StateNode = NodeCreator.CreateNode(false);
		StateNode->NodePosX = Index * YSPlaybackMigration::StateSpacingX;
		StateNode->NodePosY = YSPlaybackMigration::StateRowY;

		// 어빌리티 CDO 의 원본을 그대로 참조하면 어빌리티를 지울 때 같이 날아가므로 사본을 갖는다.
		if (SourcePlaybacks[Index] != nullptr)
		{
			StateNode->Playback = DuplicateObject<UYSAbilityPlaybackBase>(SourcePlaybacks[Index], StateNode);
			StateNode->PlaybackClass = SourcePlaybacks[Index]->GetClass();
		}

		if (StateNames.IsValidIndex(Index))
		{
			StateNode->StateName = StateNames[Index];
		}

		NodeCreator.Finalize();

		StateNodes.Add(StateNode);
	}

	if (UYSPlaybackGraphNode_Entry* EntryNode = YSPlaybackMigration::FindEntry(Graph))
	{
		EntryNode->NodePosX = -300;
		EntryNode->NodePosY = YSPlaybackMigration::StateRowY;

		UEdGraphPin* EntryOutput = EntryNode->GetOutputPin();
		UEdGraphPin* FirstInput = StateNodes[0]->GetInputPin();

		if (EntryOutput != nullptr && FirstInput != nullptr)
		{
			EntryOutput->BreakAllPinLinks();
			EntryOutput->MakeLinkTo(FirstInput);
		}
	}

	// 원본 플레이백은 PostLoad 가 옛 Transitions 를 Task 경로로 옮겨뒀다. 그 경로를 화살표로 편다.
	for (int32 Index = 0; Index < SourcePlaybacks.Num(); ++Index)
	{
		const UYSAbilityPlaybackBase* SourcePlayback = SourcePlaybacks[Index];

		if (SourcePlayback == nullptr)
		{
			continue;
		}

		// 전환마다 만들면 노드가 불어나 안 읽히므로 종료·유지 노드는 상태당 하나씩 돌려 쓴다.
		UYSPlaybackGraphNode_Exit* SharedExitNode = nullptr;
		UYSPlaybackGraphNode_Stay* SharedStayNode = nullptr;

		int32 RouteOrder = 0;

		for (int32 TaskIndex = 0; TaskIndex < SourcePlayback->Tasks.Num(); ++TaskIndex)
		{
			const UYSPlaybackTask* Task = SourcePlayback->Tasks[TaskIndex];

			if (Task == nullptr)
			{
				continue;
			}

			for (int32 OutputIndex = 0; OutputIndex < Task->OutputRoutes.Num(); ++OutputIndex)
			{
				for (const FYSPlaybackRoute& Route : Task->OutputRoutes[OutputIndex].Routes)
				{
					FGraphNodeCreator<UYSPlaybackGraphNode_Transition> NodeCreator(Graph);

					UYSPlaybackGraphNode_Transition* TransitionNode = NodeCreator.CreateNode(false);
					TransitionNode->TaskIndex = TaskIndex;
					TransitionNode->OutputIndex = OutputIndex;
					TransitionNode->TransitionConditions = Route.Conditions;
					TransitionNode->PriorityOrder = -RouteOrder;

					for (const UYSPlaybackAction* Action : Route.Actions)
					{
						if (Action != nullptr)
						{
							TransitionNode->Actions.Add(DuplicateObject<UYSPlaybackAction>(Action, TransitionNode));
						}
					}

					NodeCreator.Finalize();

					UYSPlaybackGraphNode_Base* TargetNode = nullptr;

					if (Route.Target == EYSRouteTarget::Node && StateNodes.IsValidIndex(Route.NextNodeIndex))
					{
						TargetNode = StateNodes[Route.NextNodeIndex];
					}
					else if (Route.Target == EYSRouteTarget::Stay)
					{
						if (SharedStayNode == nullptr)
						{
							FGraphNodeCreator<UYSPlaybackGraphNode_Stay> StayCreator(Graph);

							SharedStayNode = StayCreator.CreateNode(false);
							SharedStayNode->NodePosX = StateNodes[Index]->NodePosX + 120;
							SharedStayNode->NodePosY = YSPlaybackMigration::StateRowY - 220;

							StayCreator.Finalize();
						}

						TargetNode = SharedStayNode;
					}
					else
					{
						if (SharedExitNode == nullptr)
						{
							FGraphNodeCreator<UYSPlaybackGraphNode_Exit> ExitCreator(Graph);

							SharedExitNode = ExitCreator.CreateNode(false);
							SharedExitNode->NodePosX = StateNodes[Index]->NodePosX + 120;
							SharedExitNode->NodePosY = YSPlaybackMigration::StateRowY + 320;

							ExitCreator.Finalize();
						}

						TargetNode = SharedExitNode;
					}

					TransitionNode->CreateConnections(StateNodes[Index], TargetNode);

					TransitionNode->NodePosX = StateNodes[Index]->NodePosX + (YSPlaybackMigration::StateSpacingX / 2);
					TransitionNode->NodePosY = YSPlaybackMigration::StateRowY + 180 + (RouteOrder * 90);

					++RouteOrder;
				}
			}
		}
	}

	Graph.NotifyGraphChanged();

	// 사용자가 저장만 하고 닫아도 런타임 데이터가 맞도록 즉시 굽는다.
	FYSPlaybackGraphCompiler::Compile(Asset);

	return StateNodes.Num();
}

// 레거시 배열 → 그래프 일괄 이전. 이전 도구와 함께 지운다.
namespace YSPlaybackBatchMigration
{
	static const TCHAR* GraphRoot = TEXT("/Game/Blueprint/PlaybCk");

	static FString DescribeRoutes(const UYSPlaybackTask& Task)
	{
		FString Out;

		for (int32 OutputIndex = 0; OutputIndex < Task.OutputRoutes.Num(); ++OutputIndex)
		{
			for (const FYSPlaybackRoute& Route : Task.OutputRoutes[OutputIndex].Routes)
			{
				// 범위 밖 노드로 가는 경로는 런타임에서 종료와 같다.
				const bool bToNode = Route.Target == EYSRouteTarget::Node && Route.NextNodeIndex != INDEX_NONE;
				Out += FString::Printf(TEXT("[o%d %s c%d a%d]"), OutputIndex,
					bToNode ? *FString::Printf(TEXT("n%d"), Route.NextNodeIndex) : (Route.Target == EYSRouteTarget::Stay ? TEXT("stay") : TEXT("end")),
					Route.Conditions.Num(), Route.Actions.Num());
			}
		}

		return Out;
	}

	static FString DescribeNode(const UYSAbilityPlaybackBase* Node)
	{
		if (Node == nullptr)
		{
			return TEXT("null");
		}

		FString Out = FString::Printf(TEXT("%s enter%d commit%d endOnUnmatched%d"), *Node->GetClass()->GetName(),
			Node->EnterActions.Num(), Node->bCommitOnEnter, Node->bEndChainOnUnmatchedInput);

		for (const UYSPlaybackTask* Task : Node->Tasks)
		{
			Out += Task != nullptr ? FString::Printf(TEXT(" {%s %s}"), *Task->GetClass()->GetName(), *DescribeRoutes(*Task)) : TEXT(" {null}");
		}

		return Out;
	}

	// 같으면 빈 문자열, 다르면 처음 어긋난 노드.
	static FString Compare(const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& Legacy, const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& Compiled)
	{
		if (Legacy.Num() != Compiled.Num())
		{
			return FString::Printf(TEXT("노드 수 %d → %d"), Legacy.Num(), Compiled.Num());
		}

		for (int32 Index = 0; Index < Legacy.Num(); ++Index)
		{
			const FString Before = DescribeNode(Legacy[Index]);
			const FString After = DescribeNode(Compiled[Index]);

			if (Before != After)
			{
				return FString::Printf(TEXT("노드 %d: %s → %s"), Index, *Before, *After);
			}
		}

		return FString();
	}

	static UYSPlaybackGraphAsset* FindOrCreateGraph(const FString& PackagePath, const FString& AssetName)
	{
		const FString ObjectPath = FString::Printf(TEXT("%s/%s.%s"), *PackagePath, *AssetName, *AssetName);

		if (UYSPlaybackGraphAsset* Existing = LoadObject<UYSPlaybackGraphAsset>(nullptr, *ObjectPath, nullptr, LOAD_NoWarn | LOAD_Quiet))
		{
			return Existing;
		}

		UPackage* Package = CreatePackage(*FString::Printf(TEXT("%s/%s"), *PackagePath, *AssetName));
		UYSPlaybackGraphAsset* Created = NewObject<UYSPlaybackGraphAsset>(Package, *AssetName, RF_Public | RF_Standalone | RF_Transactional);
		IAssetRegistry::GetChecked().AssetCreated(Created);
		Created->MarkPackageDirty();

		return Created;
	}

	static void EnsureEdGraph(UYSPlaybackGraphAsset& Asset)
	{
		if (Asset.EdGraph != nullptr)
		{
			return;
		}

		Asset.EdGraph = FBlueprintEditorUtils::CreateNewGraph(&Asset, NAME_None, UYSPlaybackGraph::StaticClass(), UYSPlaybackGraphSchema::StaticClass());
		Asset.EdGraph->bAllowDeletion = false;
		Asset.EdGraph->GetSchema()->CreateDefaultNodesForGraph(*Asset.EdGraph);
	}

	static void Run(const TArray<FString>& Args)
	{
		const bool bApply = Args.Contains(TEXT("apply"));
		FObjectProperty* GraphProperty = FindFProperty<FObjectProperty>(UYSGameplayAbility::StaticClass(), TEXT("PlaybackGraph"));

		TArray<FAssetData> Blueprints;
		IAssetRegistry::GetChecked().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), Blueprints);

		TArray<FAssetData> ExistingGraphs;
		IAssetRegistry::GetChecked().GetAssetsByClass(UYSPlaybackGraphAsset::StaticClass()->GetClassPathName(), ExistingGraphs);

		TArray<UPackage*> PackagesToSave;
		int32 Targets = 0;
		int32 Migrated = 0;

		for (const FAssetData& Data : Blueprints)
		{
			if (Data.PackagePath.ToString().StartsWith(TEXT("/Game")) == false)
			{
				continue;
			}

			UBlueprint* Blueprint = Cast<UBlueprint>(Data.GetAsset());
			UClass* AbilityClass = Blueprint != nullptr ? Blueprint->GeneratedClass.Get() : nullptr;

			if (AbilityClass == nullptr || AbilityClass->IsChildOf(UYSGameplayAbility::StaticClass()) == false)
			{
				continue;
			}

			UYSGameplayAbility* CDO = AbilityClass->GetDefaultObject<UYSGameplayAbility>();
			const TArray<TObjectPtr<UYSAbilityPlaybackBase>>& Legacy = CDO->GetLegacyPlaybacks();

			if (GraphProperty->GetObjectPropertyValue_InContainer(CDO) != nullptr || Legacy.Num() == 0)
			{
				continue;
			}

			++Targets;

			const FString PackagePath = FString::Printf(TEXT("%s/%s"), GraphRoot, *FPaths::GetCleanFilename(Data.PackagePath.ToString()));
			const FString AssetName = Blueprint->GetName() + TEXT("_Playback");

			// 같은 어빌리티를 가리키는 지정 안 된 그래프가 이미 있으면 함께 대조해 남긴다.
			for (const FAssetData& GraphData : ExistingGraphs)
			{
				const UYSPlaybackGraphAsset* OldGraph = Cast<UYSPlaybackGraphAsset>(GraphData.GetAsset());

				if (OldGraph != nullptr && OldGraph->SourceAbility.Get() == AbilityClass && GraphData.AssetName.ToString() != AssetName)
				{
					const FString Diff = Compare(Legacy, OldGraph->Playbacks);
					UE_LOG(LogTemp, Display, TEXT("[PlaybackMigration] %s 옛 그래프 %s : %s"), *Blueprint->GetName(), *GraphData.PackageName.ToString(),
						Diff.IsEmpty() ? TEXT("구조 같음") : *Diff);
				}
			}

			if (bApply == false)
			{
				UE_LOG(LogTemp, Display, TEXT("[PlaybackMigration] 대상 %s (%d 노드) → %s/%s"), *Blueprint->GetName(), Legacy.Num(), *PackagePath, *AssetName);
				continue;
			}

			UYSPlaybackGraphAsset* Graph = FindOrCreateGraph(PackagePath, AssetName);

			if (Graph == nullptr)
			{
				UE_LOG(LogTemp, Error, TEXT("[PlaybackMigration] %s : 그래프 애셋을 만들지 못함"), *Blueprint->GetName());
				continue;
			}

			EnsureEdGraph(*Graph);
			Graph->SourceAbility = AbilityClass;
			const int32 Imported = FYSPlaybackGraphMigration::ImportFromSourceAbility(Graph);
			PackagesToSave.AddUnique(Graph->GetPackage());

			const FString Diff = Compare(Legacy, Graph->Playbacks);

			// 원래 배열과 다르면 지정하지 않는다 — 어빌리티는 레거시로 계속 돈다.
			if (Imported == 0 || Diff.IsEmpty() == false)
			{
				UE_LOG(LogTemp, Error, TEXT("[PlaybackMigration] %s : 대조 실패, 지정 안 함 — %s"), *Blueprint->GetName(), *Diff);
				continue;
			}

			CDO->Modify();
			GraphProperty->SetObjectPropertyValue_InContainer(CDO, Graph);
			CDO->MarkPackageDirty();
			PackagesToSave.AddUnique(Blueprint->GetPackage());
			++Migrated;

			UE_LOG(LogTemp, Display, TEXT("[PlaybackMigration] %s : %d 노드 이전 · 대조 같음 → %s"), *Blueprint->GetName(), Imported, *Graph->GetPathName());
		}

		const bool bSaved = PackagesToSave.Num() == 0 || UEditorLoadingAndSavingUtils::SavePackages(PackagesToSave, false);
		UE_LOG(LogTemp, Display, TEXT("[PlaybackMigration] %s 대상 %d · 이전 %d · 저장 패키지 %d (%s)"),
			bApply ? TEXT("적용") : TEXT("미리보기"), Targets, Migrated, PackagesToSave.Num(), bSaved ? TEXT("성공") : TEXT("실패"));
	}

	static FAutoConsoleCommand Command(
		TEXT("YS.Playback.MigrateLegacy"),
		TEXT("레거시 플레이백 배열만 쓰는 어빌리티를 그래프 애셋으로 옮긴다. 인자 없으면 미리보기, apply 면 생성 · 지정 · 저장."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&Run));
}

// 없앤 컨텍스트 스위치 변형 표의 백업(rows.json)을 CS 어빌리티 그래프로 옮긴다. 한 번 쓰고 지우는 도구다.
namespace YSContextSwitchGraphImport
{
	static const TCHAR* AbilityPath = TEXT("/Game/Blueprint/Ability/BP_ContextSwitchAttack.BP_ContextSwitchAttack");
	static const TCHAR* WarpTargetName = TEXT("CS_Target");

	static FGameplayTagContainer ReadTags(const FJsonObject& Row, const TCHAR* Field)
	{
		FGameplayTagContainer Tags;
		const TArray<TSharedPtr<FJsonValue>>* Values = nullptr;

		if (Row.TryGetArrayField(Field, Values))
		{
			for (const TSharedPtr<FJsonValue>& Value : *Values)
			{
				Tags.AddTag(FGameplayTag::RequestGameplayTag(FName(Value->AsString())));
			}
		}

		return Tags;
	}

	// 옛 표의 거리 칸은 비트마스크지만 거리 구간 조건은 구간 하나다. 둘 이상이면 옮기지 않는다.
	static bool AddRouteConditions(const FJsonObject& Row, FYSPlaybackRoute& Route, FString& OutError)
	{
		const FGameplayTagContainer Required = ReadTags(Row, TEXT("TargetRequiredTags"));
		const FGameplayTagContainer Excluded = ReadTags(Row, TEXT("TargetExcludedTags"));

		if (Required.IsEmpty() == false || Excluded.IsEmpty() == false)
		{
			FYSPlaybackCondition_TargetTag Condition;
			Condition.RequiredTags = Required;
			Condition.ExcludedTags = Excluded;
			Route.Conditions.Add(FInstancedStruct::Make(Condition));
		}

		const int32 HeightMask = static_cast<int32>(Row.GetNumberField(TEXT("HeightMask")));

		if (HeightMask != 0)
		{
			FYSPlaybackCondition_TargetHeight Condition;
			Condition.HeightMask = HeightMask;
			Route.Conditions.Add(FInstancedStruct::Make(Condition));
		}

		const int32 DistanceMask = static_cast<int32>(Row.GetNumberField(TEXT("DistanceMask")));

		if (DistanceMask != 0)
		{
			if (FMath::IsPowerOfTwo(DistanceMask) == false)
			{
				OutError = FString::Printf(TEXT("거리 칸에 구간이 여럿(%d)"), DistanceMask);
				return false;
			}

			FYSPlaybackCondition_TargetDistanceBand Condition;
			Condition.Band = static_cast<EYSTargetDistanceBand>(FMath::FloorLog2(DistanceMask));
			Route.Conditions.Add(FInstancedStruct::Make(Condition));
		}

		const int32 DirectionMask = static_cast<int32>(Row.GetNumberField(TEXT("DirectionMask")));

		if (DirectionMask != 0)
		{
			FYSPlaybackCondition_TargetDirection Condition;
			Condition.DirectionMask = DirectionMask;
			Condition.Reference = EYSForwardReference::PreSwitch;
			Route.Conditions.Add(FInstancedStruct::Make(Condition));
		}

		return true;
	}

	static UYSAbilityPlaybackBase* MakeMontageNode(const FJsonObject& Row)
	{
		UYSAbilityPlaybackBase* Node = NewObject<UYSAbilityPlaybackBase>(GetTransientPackage());
		UYSPlaybackTask_PlayMontage* Task = NewObject<UYSPlaybackTask_PlayMontage>(Node);

		Task->MontageSelector.InitializeAs<FYSMontageSelector_Default>();
		FYSMontageSelector_Default& Selector = Task->MontageSelector.GetMutable<FYSMontageSelector_Default>();
		Selector.DefaultMontage = TSoftObjectPtr<UAnimMontage>(FSoftObjectPath(Row.GetStringField(TEXT("Montage"))));

		Selector.MotionWarpingParam.InitializeAs<FYSMotionWarpParam_TargetFront>();
		FYSMotionWarpParam_TargetFront& Warp = Selector.MotionWarpingParam.GetMutable<FYSMotionWarpParam_TargetFront>();
		Warp.MotionWarpTargetName = WarpTargetName;

		double Value = 0.0;

		if (Row.TryGetNumberField(TEXT("WarpStopDistance"), Value))
		{
			Warp.StopDistance = static_cast<float>(Value);
		}

		if (Row.TryGetNumberField(TEXT("WarpMaxDistance"), Value))
		{
			Warp.MaxDistance = static_cast<float>(Value);
		}

		Node->Tasks.Add(Task);
		return Node;
	}

	static void Run(const TArray<FString>& Args)
	{
		const FString JsonPath = Args.Num() > 0 ? Args[0] : FPaths::ProjectSavedDir() / TEXT("Relay/backup_cs_variant/rows.json");

		FString Text;
		TSharedPtr<FJsonObject> Root;

		if (FFileHelper::LoadFileToString(Text, *JsonPath) == false || FJsonSerializer::Deserialize(TJsonReaderFactory<>::Create(Text), Root) == false || Root.IsValid() == false)
		{
			UE_LOG(LogTemp, Error, TEXT("[CSGraphImport] %s 를 읽지 못함"), *JsonPath);
			return;
		}

		UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, AbilityPath);
		UClass* AbilityClass = Blueprint != nullptr ? Blueprint->GeneratedClass.Get() : nullptr;

		if (AbilityClass == nullptr || AbilityClass->IsChildOf(UYSGameplayAbility::StaticClass()) == false)
		{
			UE_LOG(LogTemp, Error, TEXT("[CSGraphImport] %s 가 어빌리티 BP 가 아님"), AbilityPath);
			return;
		}

		UYSAbilityPlaybackBase* BranchNode = NewObject<UYSAbilityPlaybackBase>(GetTransientPackage());
		UYSPlaybackTask_Branch* Branch = NewObject<UYSPlaybackTask_Branch>(BranchNode);
		Branch->ResetRoutes();
		BranchNode->Tasks.Add(Branch);

		TArray<TObjectPtr<UYSAbilityPlaybackBase>> Playbacks = { BranchNode };
		TArray<FString> StateNames = { TEXT("분기") };

		for (const TSharedPtr<FJsonValue>& RowValue : Root->GetArrayField(TEXT("rows")))
		{
			const TSharedPtr<FJsonObject> RowObject = RowValue->AsObject();
			const TSharedPtr<FJsonObject>* Data = nullptr;

			if (RowObject.IsValid() == false || RowObject->TryGetObjectField(TEXT("data"), Data) == false)
			{
				continue;
			}

			const FString RowName = RowObject->GetStringField(TEXT("name"));

			FYSPlaybackRoute Route;
			FString Error;

			if (AddRouteConditions(**Data, Route, Error) == false)
			{
				UE_LOG(LogTemp, Error, TEXT("[CSGraphImport] 행 '%s' : %s — 중단"), *RowName, *Error);
				return;
			}

			Route.Target = EYSRouteTarget::Node;
			Route.NextNodeIndex = Playbacks.Num();
			Branch->AddRoute(0, Route);

			Playbacks.Add(MakeMontageNode(**Data));
			StateNames.Add(RowName);
		}

		if (Playbacks.Num() <= 1)
		{
			UE_LOG(LogTemp, Error, TEXT("[CSGraphImport] %s 에 행이 없음"), *JsonPath);
			return;
		}

		const FString PackagePath = FString::Printf(TEXT("%s/%s"), YSPlaybackBatchMigration::GraphRoot, *FPaths::GetCleanFilename(FPackageName::GetLongPackagePath(Blueprint->GetPackage()->GetName())));
		UYSPlaybackGraphAsset* Graph = YSPlaybackBatchMigration::FindOrCreateGraph(PackagePath, Blueprint->GetName() + TEXT("_Playback"));

		if (Graph == nullptr)
		{
			UE_LOG(LogTemp, Error, TEXT("[CSGraphImport] 그래프 애셋을 만들지 못함"));
			return;
		}

		Graph->Modify();
		YSPlaybackBatchMigration::EnsureEdGraph(*Graph);
		const int32 Imported = FYSPlaybackGraphMigration::ImportPlaybacks(Graph, Playbacks, StateNames);

		UYSGameplayAbility* CDO = AbilityClass->GetDefaultObject<UYSGameplayAbility>();
		FObjectProperty* GraphProperty = FindFProperty<FObjectProperty>(UYSGameplayAbility::StaticClass(), TEXT("PlaybackGraph"));

		CDO->Modify();
		GraphProperty->SetObjectPropertyValue_InContainer(CDO, Graph);
		CDO->MarkPackageDirty();
		Graph->MarkPackageDirty();

		const bool bSaved = UEditorLoadingAndSavingUtils::SavePackages({ Graph->GetPackage(), Blueprint->GetPackage() }, false);

		UE_LOG(LogTemp, Display, TEXT("[CSGraphImport] 노드 %d (경로 %d) → %s, 어빌리티 지정 · 저장 %s"),
			Imported, Playbacks.Num() - 1, *Graph->GetPathName(), bSaved ? TEXT("성공") : TEXT("실패"));
	}

	static FAutoConsoleCommand Command(
		TEXT("YS.Playback.ImportContextSwitchRows"),
		TEXT("변형 표 백업(rows.json)으로 BP_ContextSwitchAttack 의 그래프(즉시 분기 + 행마다 경로 · 몽타주 노드)를 만들고 저장한다. 인자: json 경로(생략하면 Saved/Relay/backup_cs_variant/rows.json)."),
		FConsoleCommandWithArgsDelegate::CreateStatic(&Run));
}
