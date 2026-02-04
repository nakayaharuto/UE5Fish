// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FishDetailWindow.generated.h"

/**
 * 
 */
UCLASS()
class UE5FISH_API UFishDetailWindow : public UUserWidget
{
	GENERATED_BODY()
public:

	//表示データを流し込む関数
	void SetDetailData(FString Name, FText Description, UTexture2D* Icon,float Record,float MinSize, float MaxSize, int32 Count);

protected:

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_FishName;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_Size;

    UPROPERTY(meta = (BindWidget))
    class UTextBlock* Text_Description;

    UPROPERTY(meta = (BindWidget))
    class UImage* Image_FishIcon;

    UPROPERTY(meta = (BindWidget))
    class UButton* Button_Back;

    virtual void NativeConstruct() override;

    UFUNCTION()
    void OnBackClicked();

    //-------SE---------
    UPROPERTY(EditAnywhere, Category = "Audio")
    USoundBase* DetailCloseSFX;

};
