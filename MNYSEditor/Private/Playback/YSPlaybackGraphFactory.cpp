// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/YSPlaybackGraphFactory.h"

#include "Ability/AbilityComponent/YSPlaybackGraphAsset.h"

UYSPlaybackGraphFactory::UYSPlaybackGraphFactory()
{
	SupportedClass = UYSPlaybackGraphAsset::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}

UObject* UYSPlaybackGraphFactory::FactoryCreateNew(UClass* InClass, UObject* InParent, FName InName, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	// EdGraph 는 에디터를 열 때 없으면 만든다 — 그래야 구버전 에셋도 같이 살아난다.
	return NewObject<UYSPlaybackGraphAsset>(InParent, InClass, InName, Flags);
}
