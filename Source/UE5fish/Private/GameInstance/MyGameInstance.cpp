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

    UE_LOG(LogTemp, Warning, TEXT("ALBUM: Registered %s. Total Caught: %d"), *Name, Data.TimesCaught);
}

void UMyGameInstance::InitializeAlbumFromTable(UDataTable* FishTable)
{
    if (!FishTable) return;

    TArray<FFishingFishData*> AllRows;
    FishTable->GetAllRows<FFishingFishData>(TEXT("Init"), AllRows);

    for (FFishingFishData* Row : AllRows)
    {
        // まだ図鑑に名前がない場合だけ、回数0の空データを入れる
        if (!GlobalFishAlbum.Contains(Row->FishName))
        {
            FFishAlbumData EmptyData;
            EmptyData.TimesCaught = 0; // これが0なら「未発見」扱い
            EmptyData.MaxSize = 0.0f;
            EmptyData.FishIcon = Row->UITexture;
            GlobalFishAlbum.Add(Row->FishName, EmptyData);
        }
    }
}

FFishingFishData* UMyGameInstance::FindFishDataInTable(FString FishName)
{
    if (!FishDataTable)
    {
        UE_LOG(LogTemp, Error, TEXT("FishDataTable is NOT set in MyGameInstance!"));
        return nullptr;
    }
    //データテーブルの全行を取得
    TArray<FFishingFishData*> AllRows;
    FishDataTable->GetAllRows<FFishingFishData>(TEXT("Searching Fish"), AllRows);

    //名前が一致するものを探す
    for (FFishingFishData* Row : AllRows)
    {
        if (Row && Row->FishName == FishName)
        {
            return Row;
        }
    }
    return nullptr;
}