// Fill out your copyright notice in the Description page of Project Settings.


#include "CaughtFish.h"
#include "Components/Image.h"    // ★ UImageクラスの定義 ★
#include "Components/Button.h"
#include "Components/TextBlock.h" // UTextBlockを使うために必要
#include "FishActor.h"    // AFishActorの定義を使うために必要
#include "GameInstance/MyGameInstance.h"
#include "Internationalization/Text.h"


bool UCaughtFish::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    if (Button_Exit)
    {
        // OnCloseButtonClicked 関数を Button_Exit の OnClicked イベントに結びつける
        Button_Exit->OnClicked.AddDynamic(this, &UCaughtFish::OnCloseButtonClicked);
    }

    return true;
}

void UCaughtFish::SetFishData(FText FishName, float Size, UTexture2D* FishImage, FText Description)
{
    // 魚のデータを使って UI テキストを更新
    // (AFishActorに FishName, SizeCm, Rarity が定義されている前提)
    if (Text_FishName)
    {
        UE_LOG(LogTemp, Log, TEXT("UI Set Name: %s"), *FishName.ToString());
        Text_FishName->SetText(FishName);
    }

    if (Text_Size)
    {
        // float を FText に変換 (例: "12.5 cm")
        FString SizeString = FString::Printf(TEXT("%.1f cm"), Size);
        Text_Size->SetText(FText::FromString(SizeString));
    }

    if (Image_FishIcon)
    {
        if (FishImage)
        {
            Image_FishIcon->SetBrushFromTexture(FishImage);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("UCaughtFish: FishImage is NULL!"));
        }
    }

    if (Text_Description)
    {
        Text_Description->SetText(Description);
    }

    if (!IsInViewport())
    {
        AddToViewport();
    }

    // 3. 【ここが本題】入力モードの切り替え
    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        // マウスカーソルを表示
        PC->bShowMouseCursor = true;

        // UI操作のみを受け付けるモードに設定
        FInputModeUIOnly InputMode;

        // 自分自身(this)を操作対象（フォーカス）として指定
        // これで「Non-Focusable」エラーを防げます
        InputMode.SetWidgetToFocus(TakeWidget());
        InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

        PC->SetInputMode(InputMode);
    }

}

void UCaughtFish::OnCloseButtonClicked()
{
    // 1. UIをビューポートから削除
    RemoveFromParent();

    // 2. 魚アクターを破棄
    if (IsValid(CurrentFishActor))
    {
        CurrentFishActor->Destroy();
        CurrentFishActor = nullptr; // 破棄後にポインタを安全にNULLに
    }

    APlayerController* PC = GetOwningPlayer();
    if (PC)
    {
        // 入力モードをゲームプレイのみに設定
        PC->SetInputMode(FInputModeGameOnly());
        // マウスカーソルを非表示にする
        PC->bShowMouseCursor = false;
    }
}
