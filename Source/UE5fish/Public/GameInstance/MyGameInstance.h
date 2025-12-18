// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "FishData.h"
#include "MyGameInstance.generated.h"

/**
 * 
 * 
 */
UCLASS()
class UE5FISH_API UMyGameInstance : public UGameInstance
{
	GENERATED_BODY()
public:
    // システム全体で共有する図鑑データ
    UPROPERTY(BlueprintReadOnly, Category = "Collection")
    TMap<FString, FFishAlbumData> GlobalFishAlbum;

    // 魚を追加する共通関数
    UFUNCTION(BlueprintCallable, Category = "Collection")
    void RegisterFishToAlbum(FString Name, float Size, UTexture2D* Icon);
};
