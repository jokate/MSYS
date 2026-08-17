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
	// 그래프 노드 외형 팩토리. 모듈이 살아 있는 동안 등록 상태를 유지한다.
	TSharedPtr<FYSPlaybackGraphNodeFactory> PlaybackNodeFactory;

	// 연결선 그리기 정책 팩토리. 상태→전환→상태를 선 하나로 합쳐 그린다.
	TSharedPtr<FYSPlaybackConnectionFactory> PlaybackConnectionFactory;
};
