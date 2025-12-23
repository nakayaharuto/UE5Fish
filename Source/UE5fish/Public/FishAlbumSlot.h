// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FishAlbumSlot.generated.h"

/**
 * 
 */
UCLASS()
class UE5FISH_API UFishAlbumSlot : public UUserWidget
{
	GENERATED_BODY()
public:
	// BP側のテキストや画像にデータを流し込む関数
	UFUNCTION(BlueprintCallable, Category = "UI")
    void OnSetFishData(FString Name, int32 CaughtCount, float MaxSize, UTexture2D* Icon);

protected:
    // meta = (BindWidget) をつけると、BP側の同名のウィジェットと自動で紐付きます
    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_FishName;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_CatchCount;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_MaxSize;

    UPROPERTY(meta = (BindWidget))
    class UImage* Image_FishIcon;
};
