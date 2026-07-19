// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AIAbility/GameplayDebuggerCategory_IAUS.h"

#include "AbilitySystemGlobals.h"
#include "YSAbilitySystemComponent.h"
#include "Ability/AIAbility/YSAIAbilityScoreFunction.h"
#include "Ability/AIAbility/YSGameplayAbility_AIBase.h"
#include "AI/Component/YSAIPerceptionComponent.h"
#include "Data/YSAbilityDataAsset.h"


FGameplayDebuggerCategory_IAUS::FGameplayDebuggerCategory_IAUS()
{
    SetDataPackReplication<FRepData>(&DataPack);
	CollectDataInterval = 0.2f;
}

void FGameplayDebuggerCategory_IAUS::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	DataPack = FRepData();
    if (IsValid(DebugActor) == false)
    {
        return;
    }

    UYSAbilitySystemComponent* YSASC = Cast<UYSAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(DebugActor));
    if (IsValid(YSASC) == false)
    {
        return;
    }

    UYSAbilityDataAsset* AbilityDataAsset = UYSAbilityDataAsset::GetDataAssetFromAbilitySystemComponent(YSASC);
    const FGameplayAbilityActorInfo* ActorInfo = YSASC->AbilityActorInfo.Get();
    if (IsValid(AbilityDataAsset) == false || ActorInfo == nullptr)
    {
        return;
    }

    AActor* OwnerActor = ActorInfo->OwnerActor.Get();
    if (IsValid(OwnerActor) == false)
    {
        return;
    }

    const TSharedPtr<FYSTargetingActorCollections> TargetingCollections = UYSAIPerceptionComponent::GetTargetingCollection(OwnerActor);

    float BestScoreFactor = 0.f;
    float BestUtilityScore = 0.f;

    for (const FYSGrantedAbilityData& GrantedAbilityData : AbilityDataAsset->GetAllAbilities())
    {
        const FGameplayAbilitySpec* AbilitySpec = YSASC->FindAbilitySpecFromClass(GrantedAbilityData.AbilityClass);
        if (AbilitySpec == nullptr)
        {
            continue;
        }

        // NonInstanced 어빌리티는 인스턴스가 없으므로 CDO로 폴백 (Evaluator와 동일한 규칙)
        const UGameplayAbility* AbilityObject = AbilitySpec->GetPrimaryInstance();
        if (IsValid(AbilityObject) == false)
        {
            AbilityObject = AbilitySpec->Ability;
        }

        const UYSGameplayAbility_AIBase* AIAbility = Cast<UYSGameplayAbility_AIBase>(AbilityObject);
        if (IsValid(AIAbility) == false)
        {
            continue;
        }

        FRepData::FAbilityScoreDebug& Entry = DataPack.Abilities.AddDefaulted_GetRef();
        Entry.AbilityName = GetNameSafe(AIAbility->GetClass());
        Entry.Threshold = AIAbility->UtilityScoreThreshold;
        Entry.bIsActive = AbilitySpec->IsActive();
        Entry.ScoreFactor = AIAbility->GetUtilityScoreFactor(ActorInfo, TargetingCollections.Get());
        Entry.UtilityScore = AIAbility->BaseUtilityScore * Entry.ScoreFactor;
        Entry.bCanActivate = AIAbility->CanActivateAbility(AbilitySpec->Handle, ActorInfo);

        for (const UYSAIAbilityScoreFunctionBase* Function : AIAbility->UtilityScore)
        {
            if (IsValid(Function) == false)
            {
                continue;
            }

            FRepData::FScoreFactorDebug& FactorEntry = Entry.Factors.AddDefaulted_GetRef();
#if WITH_EDITOR
            FactorEntry.FunctionName = Function->GetClass()->GetDisplayNameText().ToString();
#else
            FactorEntry.FunctionName = Function->GetClass()->GetName();
#endif
            FactorEntry.Factor = Function->GetScoreFactor(ActorInfo, TargetingCollections.Get());
        }

        // Evaluator는 활성 중인 어빌리티를 후보에서 제외한다 (표시는 하되 베스트 판정에서만 뺀다)
        if (Entry.bIsActive)
        {
            continue;
        }

        // Evaluator와 동일한 선택 규칙: 팩터 우선, 동률이면 가중 점수로 판정
        const int32 EntryIndex = DataPack.Abilities.Num() - 1;
        if (Entry.ScoreFactor > BestScoreFactor)
        {
            BestScoreFactor = Entry.ScoreFactor;
            BestUtilityScore = Entry.ScoreFactor;
            DataPack.BestAbilityIndex = EntryIndex;
        }
        else if (Entry.ScoreFactor == BestScoreFactor && Entry.UtilityScore > BestUtilityScore)
        {
            BestUtilityScore = Entry.UtilityScore;
            DataPack.BestAbilityIndex = EntryIndex;
        }
    }
}

void FGameplayDebuggerCategory_IAUS::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
    if (DataPack.Abilities.Num() == 0)
    {
        CanvasContext.Printf(TEXT("{grey}No IAUS data (check ASC / AbilityDataAsset on debug target)"));
        return;
    }

    for (int32 Index = 0; Index < DataPack.Abilities.Num(); ++Index)
    {
        const FRepData::FAbilityScoreDebug& Entry = DataPack.Abilities[Index];

        const bool bIsBest = (Index == DataPack.BestAbilityIndex);
        const TCHAR* NameColor = bIsBest ? TEXT("{green}") : TEXT("{white}");
        const TCHAR* ScoreColor = (Entry.UtilityScore > Entry.Threshold) ? TEXT("{yellow}") : TEXT("{grey}");

        CanvasContext.Printf(TEXT("%s%s{white}  Score: %s%.1f{white}  (Factor %.3f)  Threshold: %.1f  %s%s%s"),
            NameColor, *Entry.AbilityName,
            ScoreColor, Entry.UtilityScore,
            Entry.ScoreFactor,
            Entry.Threshold,
            Entry.bCanActivate ? TEXT("{green}[Activatable]") : TEXT("{red}[Blocked]"),
            Entry.bIsActive ? TEXT(" {cyan}[Active]") : TEXT(""),
            bIsBest ? TEXT(" {green}[BEST]") : TEXT(""));

        for (const FRepData::FScoreFactorDebug& FactorEntry : Entry.Factors)
        {
            // 팩터 0 = 거부권(veto)이므로 빨간색으로 강조
            const TCHAR* FactorColor = (FactorEntry.Factor <= KINDA_SMALL_NUMBER) ? TEXT("{red}") : TEXT("{white}");
            CanvasContext.Printf(TEXT("    %s%s: %.3f"), FactorColor, *FactorEntry.FunctionName, FactorEntry.Factor);
        }
    }
}

TSharedRef<FGameplayDebuggerCategory> FGameplayDebuggerCategory_IAUS::MakeInstance()
{
	return MakeShareable(new FGameplayDebuggerCategory_IAUS());
}

void FGameplayDebuggerCategory_IAUS::FRepData::Serialize(FArchive& Ar)
{
    Ar << BestAbilityIndex;

    int32 NumAbilities = Abilities.Num();
    Ar << NumAbilities;
    if (Ar.IsLoading())
    {
        Abilities.SetNum(NumAbilities);
    }

    for (int32 Idx = 0; Idx < NumAbilities; Idx++)
    {
        FAbilityScoreDebug& Entry = Abilities[Idx];
        Ar << Entry.AbilityName;
        Ar << Entry.UtilityScore;
        Ar << Entry.ScoreFactor;
        Ar << Entry.Threshold;
        Ar << Entry.bCanActivate;
        Ar << Entry.bIsActive;

        int32 NumFactors = Entry.Factors.Num();
        Ar << NumFactors;
        if (Ar.IsLoading())
        {
            Entry.Factors.SetNum(NumFactors);
        }

        for (int32 FactorIdx = 0; FactorIdx < NumFactors; FactorIdx++)
        {
            Ar << Entry.Factors[FactorIdx].FunctionName;
            Ar << Entry.Factors[FactorIdx].Factor;
        }
    }
}
