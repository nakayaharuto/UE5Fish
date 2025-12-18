// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/MyGameInstance.h"

void UMyGameInstance::RegisterFishToAlbum(FString Name, float Size, UTexture2D* Icon)
{
    // 1. すでに図鑑にあるか確認、なければ新規作成
    FFishAlbumData& Data = GlobalFishAlbum.FindOrAdd(Name);

    // 2. 釣った回数を増やす
    Data.TimesCaught++;

    // 3. 最大サイズの更新
    if (Size > Data.MaxSize)
    {
        Data.MaxSize = Size;
    }

    // 4. アイコンの設定（初回のみ、または毎回上書き）
    if (Icon)
    {
        Data.FishIcon = Icon;
    }

    UE_LOG(LogTemp, Warning, TEXT("System: RegisterFishToAlbum - %s (Size: %.1f)"), *Name, Size);
}