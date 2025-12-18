// Fill out your copyright notice in the Description page of Project Settings.


#include "FishAlbumSlot.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"

void UFishAlbumSlot::OnSetFishData(const FString& Name, int32 Count, float MaxSize, UTexture2D* Icon)
{
    if (Text_FishName) Text_FishName->SetText(FText::FromString(Name));

    if (Text_CatchCount)
        Text_CatchCount->SetText(FText::FromString(FString::Printf(TEXT("%d •C"), Count)));

    if (Text_MaxSize)
        Text_MaxSize->SetText(FText::FromString(FString::Printf(TEXT("%.1f cm"), MaxSize)));

    if (Image_FishIcon && Icon)
        Image_FishIcon->SetBrushFromTexture(Icon);
}