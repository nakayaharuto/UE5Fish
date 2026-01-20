// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FishAlbumSlot.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnFishSlotClicked, FString, FishName);

UCLASS()
class UE5FISH_API UFishAlbumSlot : public UUserWidget
{
	GENERATED_BODY()
public:
	// BP側のテキストや画像にデータを流し込む関数
	UFUNCTION(BlueprintCallable, Category = "UI")
    void OnSetFishData(FString Name, int32 CaughtCount, float MaxSize, UTexture2D* Icon);

    //クリックイベント
    UPROPERTY(BlueprintAssignable, Category = "Events")
    FOnFishSlotClicked OnSlotClicked;

    // 詳細画面のクラス（WBP_FishDetailWindow）をセットするための変数
    UPROPERTY(EditAnywhere, Category = "UI")
    TSubclassOf<class UFishDetailWindow> DetailWindowClass;

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

    UPROPERTY(meta = (BindWidget))
    class UButton* Button_FishSelect;

    virtual void NativeConstruct() override;

    UFUNCTION()
    void InternalOnClicked();

    // 内部保持用
    FString MyFishName;

};
