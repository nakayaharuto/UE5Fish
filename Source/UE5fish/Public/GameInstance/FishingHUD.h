// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "FishData.h"
#include "FishingHUD.generated.h"

/**
 * 
 */
UCLASS()
class UE5FISH_API AFishingHUD : public AHUD
{
	GENERATED_BODY()

public:
    // TAB‚ÅŒÄ‚Î‚ê‚éUIƒgƒOƒ‹ŠÖ”
    void ToggleFishAlbum();

    //}ŠÓ‚ğŠJ‚¢‚½‚Ìˆ—
    void RefreshAlbum(UUserWidget* AlbumUI);

protected:
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UUserWidget> AlbumWidgetClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UUserWidget> SlotWidgetClass;


    UPROPERTY()
    class UUserWidget* AlbumWidget;
};
