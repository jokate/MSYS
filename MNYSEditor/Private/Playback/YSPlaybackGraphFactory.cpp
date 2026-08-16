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
	// EdGraph 는 여기서 만들지 않는다.
	// 에디터를 열 때 없으면 만드는 쪽이 구버전 에셋도 같이 살린다.
	return NewObject<UYSPlaybackGraphAsset>(InParent, InClass, InName, Flags);
}
