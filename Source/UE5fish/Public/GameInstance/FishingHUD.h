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
    // TABで呼ばれるUIトグル関数
    void ToggleFishAlbum();

    //図鑑を開いた時の処理
    void RefreshAlbum(UUserWidget* AlbumUI);

    // スロットから呼ばれる入り口
    void ShowFishDetail(FString Name, int32 Count, float Size, float MinSize, float MaxSize, UTexture2D* Icon);

protected:
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UUserWidget> AlbumWidgetClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UUserWidget> SlotWidgetClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UFishDetailWindow> DetailWindowClass;

    UPROPERTY()
    class UUserWidget* AlbumWidget;
};
