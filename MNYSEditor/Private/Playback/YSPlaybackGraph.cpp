// Fill out your copyright notice in the Description page of Project Settings.


#include "Playback/YSPlaybackGraph.h"

#include "Ability/AbilityComponent/YSPlaybackGraphAsset.h"

UYSPlaybackGraphAsset* UYSPlaybackGraph::GetPlaybackAsset() const
{
	return Cast<UYSPlaybackGraphAsset>(GetOuter());
}
