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

    //外部から呼ぶための音楽切り替え関数
    void UpdateFishingAudio(FString State);

    virtual void BeginPlay() override;

private:
    void PlayBGM(USoundBase* NewSound, bool bLoop);


protected:
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UUserWidget> AlbumWidgetClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UUserWidget> SlotWidgetClass;

    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UFishDetailWindow> DetailWindowClass;

    UPROPERTY()
    class UUserWidget* AlbumWidget;

    //===========BGM管理用========
    UPROPERTY()
    UAudioComponent* MainBGMComponent;

    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* MainBGM;

    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* FishingBGM;

    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* SuccessSFX;

    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* FailSFX;

    UPROPERTY(EditAnywhere,Category = "Audio")
    USoundBase* AlbumOpenSFX;

    UPROPERTY(EditAnywhere,Category = "Audio")
    USoundBase* AlbumCloseSFX;

    UPROPERTY(EditAnywhere,Category = "Audio")
    USoundBase* DetailOpenSFX;

    
};
