// Fill out your copyright notice in the Description page of Project Settings.


#include "AttackableActor/YSAttackSpawner.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "Components/ArrowComponent.h"
#include "General/YSStruct.h"
#include "Library/YSBlueprintFunctionLibrary.h"


// Sets default values
AYSAttackSpawner::AYSAttackSpawner()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}

void AYSAttackSpawner::OnActivate_Implementation()
{
	TrySpawnActor();
}

void AYSAttackSpawner::SpawnActorByConfig(FYSSpawnActorConfig SpawnConfig)
{
	++SpawnCount;

	AActor* OwnerForPolicy = OwnerActor.IsValid() ? OwnerActor.Get() : this;

	UYSBlueprintFunctionLibrary::SpawnByConfig(
		this,
		SpawnConfig,
		OwnerForPolicy,
		TargetActor.Get(),
		/*AttachParent*/ this);

	if ( SpawnCount >= SpawnActorConfigs.Num() )
	{
		Destroy();
	}
}

void AYSAttackSpawner::TrySpawnActor()
{
	for ( const FYSSpawnActorConfig& SpawnConfig : SpawnActorConfigs )
	{
		if (SpawnConfig.SpawnDelay > 0.f )
		{
			FTimerHandle TempSpawnTimerHandle;
			GetWorldTimerManager().SetTimer(TempSpawnTimerHandle, FTimerDelegate::CreateUObject(this, &AYSAttackSpawner::SpawnActorByConfig, SpawnConfig), SpawnConfig.SpawnDelay, false);			
		}
		else
		{
			SpawnActorByConfig(SpawnConfig);
		}
	}
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

void AYSAttackSpawner::BeginPlay()
{
	Super::BeginPlay();
	
	ProcessActivationType();
}

void AYSAttackSpawner::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	DeprocessActivationType();
	
	Super::EndPlay(EndPlayReason);
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

		if (Config.PositionPolicy == EYSPositionPolicy::UseRelativeOffset)
		{
			Arrow->SetRelativeLocation(Config.RelativeOffset);
			Arrow->SetRelativeRotation(Config.RelativeRotator);
		}

		SpawnPreviewArrows.Add(Arrow);
	}
}
#endif

