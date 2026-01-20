// Fill out your copyright notice in the Description page of Project Settings.


#include "FishDetailWindow.h"
#include "Components/TextBlock.h"
#include "Components/Image.h"
#include "Components/Button.h"

void UFishDetailWindow::NativeConstruct()
{
    Super::NativeConstruct();

    if (Button_Back)
    {
        Button_Back->OnClicked.AddDynamic(this, &UFishDetailWindow::OnBackClicked);
    }
}

void UFishDetailWindow::SetDetailData(FString Name, FText Description, UTexture2D* Icon, float MaxSize, int32 Count)
{
    if (Text_FishName)
    {
        Text_FishName->SetText(FText::FromString(Name));
    }

    if (Text_Description)
    {
        Text_Description->SetText(Description);
    }

    if (Image_FishIcon && Icon)
    {
        Image_FishIcon->SetBrushFromTexture(Icon);
    }

    // 必要に応じて最大サイズや釣った回数も表示
    // 例: FString::Printf(TEXT("記録: %.1f cm / 釣った回数: %d 回"), MaxSize, Count);
}

void UFishDetailWindow::OnBackClicked()
{
    // 詳細画面を閉じる（自分を消去する）
    RemoveFromParent();
}