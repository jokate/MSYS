// Fill out your copyright notice in the Description page of Project Settings.


#include "Ability/AIAbility/GameplayDebuggerCategory_IAUS.h"

#include "AbilitySystemGlobals.h"
#include "YSAbilitySystemComponent.h"
#include "Ability/AIAbility/YSAIAbilityScoreFunction.h"
#include "Ability/AIAbility/YSGameplayAbility_AIBase.h"


FGameplayDebuggerCategory_IAUS::FGameplayDebuggerCategory_IAUS()
{
    SetDataPackReplication<FRepData>(&DataPack);
	CollectDataInterval = 0.2f;
}

void FGameplayDebuggerCategory_IAUS::CollectData(APlayerController* OwnerPC, AActor* DebugActor)
{
	DataPack = FRepData();

    UYSAbilitySystemComponent* YSASC = Cast<UYSAbilitySystemComponent>(UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(DebugActor));
    const FYSAIAbilitySelection Selection = UYSGameplayAbility_AIBase::SelectAbility(YSASC);

    for (const FYSAIAbilityScore& Score : Selection.Scores)
    {
        FRepData::FAbilityScoreDebug& Entry = DataPack.Abilities.AddDefaulted_GetRef();
        Entry.AbilityName = GetNameSafe(Score.Ability->GetClass());
        Entry.Threshold = Score.Ability->UtilityScoreThreshold;
        Entry.bIsActive = Score.bIsActive;
        Entry.ScoreFactor = Score.Factor;
        Entry.UtilityScore = Score.Score;
        Entry.bCanActivate = Score.bCanActivate;

        for (const TPair<const UYSAIAbilityScoreFunctionBase*, float>& Factor : Score.Factors)
        {
            FRepData::FScoreFactorDebug& FactorEntry = Entry.Factors.AddDefaulted_GetRef();
#if WITH_EDITOR
            FactorEntry.FunctionName = Factor.Key->GetClass()->GetDisplayNameText().ToString();
#else
            FactorEntry.FunctionName = Factor.Key->GetClass()->GetName();
#endif
            FactorEntry.Factor = Factor.Value;
        }
    }

    DataPack.BestAbilityIndex = Selection.BestIndex;
}

void FGameplayDebuggerCategory_IAUS::DrawData(APlayerController* OwnerPC, FGameplayDebuggerCanvasContext& CanvasContext)
{
    if (DataPack.Abilities.Num() == 0)
    {
        CanvasContext.Printf(TEXT("{grey}No IAUS data (check ASC / AbilityDataAsset / perception targets on debug target)"));
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
