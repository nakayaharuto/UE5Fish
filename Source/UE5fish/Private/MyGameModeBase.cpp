// Fill out your copyright notice in the Description page of Project Settings.


#include "MyGameModeBase.h"
#include "GameInstance/FishingHUD.h"
#include "MyCharacter/MyCharacter.h"
#include "UObject/ConstructorHelpers.h"

AMyGameModeBase::AMyGameModeBase()
{
	static ConstructorHelpers::FClassFinder<AHUD> HUDBPClass(TEXT("/Game/BP_Character/BP_FishingHUD"));

	if (HUDBPClass.Class != nullptr)
	{
		HUDClass = HUDBPClass.Class;
	}

	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(TEXT("/Game/BP_Character/BP_MyCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}