// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleGameMode.h"
#include "Blueprint/UserWidget.h"
#include "MyPlayerController.h"

ATitleGameMode::ATitleGameMode()
{
	PlayerControllerClass = AMyPlayerController::StaticClass();
	//スペクテーターモード
	DefaultPawnClass = nullptr;
}

void ATitleGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (TitleWidgetClass)
	{
		CurrentWidget = CreateWidget<UUserWidget>(GetWorld(), TitleWidgetClass);
		if (CurrentWidget)
		{
			CurrentWidget->AddToViewport();
		}
	}

}
