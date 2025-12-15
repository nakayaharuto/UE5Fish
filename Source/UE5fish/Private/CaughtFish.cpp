// Fill out your copyright notice in the Description page of Project Settings.


#include "CaughtFish.h"
#include "Components/Image.h"    // ★ UImageクラスの定義 ★
#include "Components/Button.h"
#include "Components/TextBlock.h" // UTextBlockを使うために必要
#include "FishActor.h"            // AFishActorの定義を使うために必要
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

void UCaughtFish::SetFishData(AFishActor* Fish)
{
    if (!IsValid(Fish))
    {
        UE_LOG(LogTemp, Error, TEXT("UCaughtFish::SetFishData failed: Passed FishActor is invalid/NULL."));
        // 処理を中断し、クラッシュを防ぐ
        return;
    }

    CurrentFishActor = Fish; // 魚アクターを保持

    // 魚のデータを使って UI テキストを更新
    // (AFishActorに FishName, SizeCm, Rarity が定義されている前提)

    if (Text_FishName)
    {
        UE_LOG(LogTemp, Warning, TEXT("Attempting to set Text. FishName is: %s"), *Fish->FishName);
        Text_FishName->SetText(FText::FromString(Fish->FishName));
    }

    if (Text_Size)
    {
        // float を FText に変換 (例: "12.5 cm")
        FString SizeString = FString::Printf(TEXT("%.1f cm"), Fish->SizeCm);
        Text_Size->SetText(FText::FromString(SizeString));
    }

    if (Text_Rarity)
    {
        // Rarity を FText に変換
        FString RarityString = FString::Printf(TEXT("レア度: %d"), Fish->FishRarity);
        Text_Rarity->SetText(FText::FromString(RarityString));
    }

    if (Image_FishIcon && Fish->UITexture)
    {
        Image_FishIcon->SetBrushFromTexture(Fish->UITexture);
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