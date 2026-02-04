// Fill out your copyright notice in the Description page of Project Settings.

#include "GameInstance/FishingHUD.h"
#include "Blueprint/UserWidget.h"
#include "Components/ScrollBox.h"
#include "Kismet/GameplayStatics.h"
#include "Components/AudioComponent.h"    // UAudioComponentの操作に必要
#include "Sound/SoundBase.h"              // USoundBaseの定義に必要
#include "TimerManager.h"                 // FTimerHandleやタイマー管理に必要
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
        AlbumWidget = CreateWidget<UUserWidget>(GetWorld(), AlbumWidgetClass);
    }

    if (AlbumWidget)
    {
        if (!AlbumWidget->IsInViewport())
        {
            // 図鑑を開く
            AlbumWidget->AddToViewport();
            UGameplayStatics::PlaySound2D(this, AlbumOpenSFX, 0.5f);

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
            // 図鑑を閉じる
            AlbumWidget->RemoveFromParent();
            UGameplayStatics::PlaySound2D(this, AlbumCloseSFX, 0.4f);

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

void AFishingHUD::BeginPlay()
{
    Super::BeginPlay();

    // ゲーム開始時に通常のBGMを再生する
    UpdateFishingAudio(TEXT("Normal"));
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

        FFishingFishData* TableData = GI->FindFishDataInTable(Elem.Key);

        float TableMin = TableData ? TableData->MinSize : 0.0f;
        float TableMax = TableData ? TableData->MaxSize : 0.0f;

        UFishAlbumSlot* NewSlot = CreateWidget<UFishAlbumSlot>(GetWorld(), SlotWidgetClass);
        if (NewSlot)
        {
            NewSlot->OnSetFishData(Elem.Key, Elem.Value.TimesCaught, Elem.Value.MaxSize, TableMin,TableMax,Elem.Value.FishIcon);
            FishList->AddChild(NewSlot);
        }
    }
}

void AFishingHUD::ShowFishDetail(FString Name, int32 Count, float Size, float MinSize,float MaxSize,UTexture2D* Icon)
{
    if (!DetailWindowClass) return;

    // ここで1回だけ生成してViewportに出す
    UFishDetailWindow* DetailWin = CreateWidget<UFishDetailWindow>(GetWorld(), DetailWindowClass);
    if (DetailWin)
    {
        UGameplayStatics::PlaySound2D(this, DetailOpenSFX, 0.6f);

        UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance());
        FFishingFishData* TableData = (GI) ? GI->FindFishDataInTable(Name) : nullptr;

        FText Desc = (TableData) ? TableData->FishDescription : FText::FromString(TEXT("説明なし"));

        DetailWin->SetDetailData(Name, Desc, Icon, Size, MinSize, MaxSize, Count);
        DetailWin->AddToViewport(100); // 重なり順を高く設定
    }
}

//===========BGM==========
void AFishingHUD::PlayBGM(USoundBase* NewSound, bool bLoop)
{
    if (MainBGMComponent)
    {
        MainBGMComponent->Stop();
        MainBGMComponent = nullptr;
    }

    if (NewSound)
    {
        MainBGMComponent = UGameplayStatics::SpawnSound2D(this, NewSound);
        if (MainBGMComponent)
        {
            MainBGMComponent->SetVolumeMultiplier(0.3f);//音量調節
            MainBGMComponent->bAutoDestroy = true; // 終わったら自動消滅
        }
    }
}

void AFishingHUD::UpdateFishingAudio(FString State)
{
    if (State == "Normal")
    {
        PlayBGM(MainBGM, true);
    }
    else if (State == "Hit")
    {
        PlayBGM(FishingBGM, true);
    }
    else if (State == "Success") {
        PlayBGM(nullptr, false); // BGM停止
        UGameplayStatics::PlaySound2D(this, SuccessSFX);
        // 3秒後にNormalに戻す（簡易版）
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, [this]() { UpdateFishingAudio("Normal"); }, 3.0f, false);
    }
    else if (State == "Fail") {
        PlayBGM(nullptr, false);
        UGameplayStatics::PlaySound2D(this, FailSFX);
        FTimerHandle TimerHandle;
        GetWorldTimerManager().SetTimer(TimerHandle, [this]() { UpdateFishingAudio("Normal"); }, 3.0f, false);
    }
}