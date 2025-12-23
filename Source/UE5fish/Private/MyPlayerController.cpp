// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPlayerController.h"

void AMyPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// マウスカーソルを表示する（Actorなので、自分自身のプロパティを操作できる）
	bShowMouseCursor = true;

	// 入力モードをUIのみに設定
	FInputModeUIOnly InputMode;
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
}