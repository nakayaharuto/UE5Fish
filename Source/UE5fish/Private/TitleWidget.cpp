// Fill out your copyright notice in the Description page of Project Settings.


#include "TitleWidget.h"
#include "Kismet/GameplayStatics.h"

bool UTitleWidget::Initialize()
{
	if (!Super::Initialize()) return false;

	if (StartButton)
	{
		StartButton->OnClicked.AddDynamic(this, &UTitleWidget::OnStartButtonClicked);
	}
	return true;
}

void UTitleWidget::OnStartButtonClicked()
{
	UE_LOG(LogTemp, Warning, TEXT("—ˆ‚Ä‚Ü‚·‚æ‚ËH"));
	// "MainMap" ‚ÍˆÚ“®æ‚ÌƒŒƒxƒ‹–¼‚É‘‚«Š·‚¦‚Ä‚­‚¾‚³‚¢
	UGameplayStatics::OpenLevel(this, FName("Untitled"));
}