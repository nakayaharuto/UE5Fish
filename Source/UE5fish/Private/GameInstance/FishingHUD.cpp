// Fill out your copyright notice in the Description page of Project Settings.

#include "GameInstance/FishingHUD.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "FishAlbumSlot.h"
#include "GameFramework/PlayerController.h" // GetOwningPlayerController用
#include "GameInstance/MyGameInstance.h" // GameInstanceのデータにアクセスする場合

void AFishingHUD::ToggleFishAlbum()
{
    if (AlbumWidgetClass == nullptr)
    {
        UE_LOG(LogTemp, Error, TEXT("AlbumWidgetClass is NULL! Please set it in BP_FishingHUD."));
        return;
    }

    if (!AlbumWidget)
    {
        UE_LOG(LogTemp, Warning, TEXT("Creating Widget..."));
        AlbumWidget = CreateWidget<UUserWidget>(GetWorld(), AlbumWidgetClass);
    }

    if (AlbumWidget)
    {
        if (!AlbumWidget->IsInViewport())
        {
            AlbumWidget->AddToViewport();

            APlayerController* PC = GetOwningPlayerController();
            PC->bShowMouseCursor = true;

            // FInputModeGameAndUI を使い、Focus先をウィジェットに指定する
            FInputModeGameAndUI InputMode;
            InputMode.SetWidgetToFocus(AlbumWidget->TakeWidget()); // ウィジェットにフォーカスを当てる
            InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

            PC->SetInputMode(InputMode);
        }
        else
        {
            AlbumWidget->RemoveFromParent();
            GetOwningPlayerController()->bShowMouseCursor = false;
            GetOwningPlayerController()->SetInputMode(FInputModeGameOnly());
        }
    }

    if (AlbumWidget && AlbumWidget->IsInViewport())
    {
        // 図鑑が表示されたら、最新のデータをリストに反映させる
        RefreshAlbum(AlbumWidget);
    }
}

//図鑑を開いた時の処理
void AFishingHUD::RefreshAlbum(UUserWidget* AlbumUI)
{
    // 1. GameInstanceからデータを取得
    UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
    if (!GI || !SlotWidgetClass || !AlbumUI) return;

    // 現在のデータ数を確認
    //UE_LOG(LogTemp, Warning, TEXT("HUD: RefreshAlbum開始。現在の図鑑データ数: %d"), GI->GlobalFishAlbum.Num());

    UScrollBox* FishList = Cast<UScrollBox>(AlbumUI->GetWidgetFromName(TEXT("ScrollBox_FishList")));
    if (!FishList)
    {
        //UE_LOG(LogTemp, Error, TEXT("HUD: ScrollBox_FishList NOT FOUND in Widget!"));
        return;
    }

    FishList->ClearChildren();

    //UE_LOG(LogTemp, Warning, TEXT("HUD: Refreshing Album. Data Count: %d"), GI->GlobalFishAlbum.Num());

    // 3. データの数だけスロットを作成して流し込む
    for (auto& Elem : GI->GlobalFishAlbum)
    {
        // ★重要：ここがログに出るか確認
        //UE_LOG(LogTemp, Warning, TEXT("HUD: Creating Slot for %s"), *Elem.Key);

        UFishAlbumSlot* NewSlot = CreateWidget<UFishAlbumSlot>(GetWorld(), SlotWidgetClass);
        if (NewSlot)
        {
            NewSlot->OnSetFishData(Elem.Key, Elem.Value.TimesCaught, Elem.Value.MaxSize, Elem.Value.FishIcon);
            FishList->AddChild(NewSlot);
        }
    }
}