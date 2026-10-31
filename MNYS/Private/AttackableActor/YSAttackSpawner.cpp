// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSAttackSpawner.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/ArrowComponent.h"
#include "General/YSStruct.h"
#include "Library/YSBlueprintFunctionLibrary.h"


AYSAttackSpawner::AYSAttackSpawner()
{
	PrimaryActorTick.bCanEverTick = false;
}

void AYSAttackSpawner::OnActivate_Implementation()
{
	Super::OnActivate_Implementation();
	TrySpawnActor();
}

void AYSAttackSpawner::TrySpawnActor()
{
	float LastSpawnDelay = 0.f;

	for ( const FYSSpawnActorConfig& SpawnConfig : SpawnActorConfigs )
	{
		// 위치 기준은 스포너 자신. 공격자는 스포너의 프록시(GetDamageInstigator)로 체인을 따라 넘어간다.
		UYSBlueprintFunctionLibrary::SpawnByConfig(
			this,
			SpawnConfig,
			FYSTransformPolicyContext(this, TargetActor.Get()),
			/*AttachParent*/ this, HitContext);

		LastSpawnDelay = FMath::Max(LastSpawnDelay, SpawnConfig.SpawnDelay);
	}

	// 지연 스폰의 어태치 · 체인 기준이라 마지막 스폰까지 남는다. 같은 시각이면 스폰이 먼저 돌도록 조금 늦춘다.
	ReturnToPoolAfter(LastSpawnDelay > 0.f ? LastSpawnDelay + KINDA_SMALL_NUMBER : 0.f);
}


#if WITH_EDITOR
void AYSAttackSpawner::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);

	if ( PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AYSAttackSpawner, SpawnActorConfigs ))
	{
		SpawnActorConfigs.StableSort([](const FYSSpawnActorConfig& A, const FYSSpawnActorConfig& B)
		{
			return A.SpawnDelay < B.SpawnDelay;
		});
	}

	_RefreshSpawnPreview();
}

void AYSAttackSpawner::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	_RefreshSpawnPreview();
}

void AYSAttackSpawner::_RefreshSpawnPreview()
{
	for (UArrowComponent* Arrow : SpawnPreviewArrows)
	{
		if (IsValid(Arrow))
		{
			Arrow->DestroyComponent();
		}
	}
	SpawnPreviewArrows.Empty();

	static const FLinearColor Colors[] = {
		FLinearColor::Red,
		FLinearColor::Green,
		FLinearColor::Blue,
		FLinearColor::Yellow,
		FLinearColor(1.f, 0.5f, 0.f),
		FLinearColor(0.5f, 0.f, 1.f),
		FLinearColor(0.f, 1.f, 1.f),
		FLinearColor(1.f, 0.f, 0.5f),
	};

	for (int32 i = 0; i < SpawnActorConfigs.Num(); ++i)
	{
		const FYSSpawnActorConfig& Config = SpawnActorConfigs[i];

		UArrowComponent* Arrow = NewObject<UArrowComponent>(this, *FString::Printf(TEXT("SpawnPreview_%d"), i));
		Arrow->SetupAttachment(SceneRoot);
		Arrow->RegisterComponent();
		Arrow->bIsEditorOnly = true;
		Arrow->SetArrowColor(Colors[i % UE_ARRAY_COUNT(Colors)]);
		Arrow->ArrowSize = 0.75f;
		Arrow->ArrowLength = 80.f;

		const FTransform PreviewTransform = Config.TransformPolicy.GetFinalTransform(FYSTransformPolicyContext(this));
		Arrow->SetWorldLocationAndRotation(PreviewTransform.GetLocation(), PreviewTransform.GetRotation());

		SpawnPreviewArrows.Add(Arrow);
	}
}
#endif

