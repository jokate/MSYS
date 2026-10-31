// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Modules/ModuleInterface.h"

class FYSPlaybackGraphNodeFactory;
class FYSPlaybackConnectionFactory;

class FMNYSEditorModule : public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	TSharedPtr<FYSPlaybackGraphNodeFactory> PlaybackNodeFactory;

	TSharedPtr<FYSPlaybackConnectionFactory> PlaybackConnectionFactory;
};
