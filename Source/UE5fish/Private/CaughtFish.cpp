// Fill out your copyright notice in the Description page of Project Settings.


#include "CaughtFish.h"


bool UCaughtFish::Initialize()
{
    bool Success = Super::Initialize();
    if (!Success) return false;

    return true;
}

void UCaughtFish::SetFishData(AFishActor* CaughtFish)
{
    if (!CaughtFish) return;

    // 魚のデータを使って UI テキストを更新
    // (AFishActorに FishName, SizeCm, Rarity が定義されている前提)

    if (Text_FishName)
    {
        Text_FishName->SetText(FText::FromString(CaughtFish->FishName));
    }

    if (Text_Size)
    {
        // float を FText に変換 (例: "12.5 cm")
        FString SizeString = FString::Printf(TEXT("%.1f cm"), CaughtFish->SizeCm);
        Text_Size->SetText(FText::FromString(SizeString));
    }

    if (Text_Rarity)
    {
        // Rarity を FText に変換
        FString RarityString = FString::Printf(TEXT("レア度: %d"), CaughtFish->Rarity);
        Text_Rarity->SetText(FText::FromString(RarityString));
    }
}