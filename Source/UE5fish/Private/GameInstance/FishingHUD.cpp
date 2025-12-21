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
}

//図鑑を開いた時の処理
void AFishingHUD::RefreshAlbum(UUserWidget* AlbumUI)
{
    // 1. GameInstanceからデータを取得
    UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
    if (!GI || !SlotWidgetClass || !AlbumUI) return;

    // 2. BP側のScrollBoxを探す（名前で検索）
    UScrollBox* FishList = Cast<UScrollBox>(AlbumUI->GetWidgetFromName(TEXT("ScrollBox_FishList")));
    if (!FishList) return;

    FishList->ClearChildren(); // 一旦中身を空にする

    // 3. データの数だけスロットを作成して流し込む
    for (auto& Elem : GI->GlobalFishAlbum)
    {
        UFishAlbumSlot* NewSlot = CreateWidget<UFishAlbumSlot>(GetWorld(), SlotWidgetClass);
        if (NewSlot)
        {
            // C++の関数を呼んでデータを流し込む！
            NewSlot->OnSetFishData(Elem.Key, Elem.Value.TimesCaught, Elem.Value.MaxSize, Elem.Value.FishIcon);

            // ScrollBoxに追加
            FishList->AddChild(NewSlot);
        }
    }
}