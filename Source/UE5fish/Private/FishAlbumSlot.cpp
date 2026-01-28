// Fill out your copyright notice in the Description page of Project Settings.


#include "FishAlbumSlot.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "FishDetailWindow.h" // 詳細画面のクラス
#include "GameInstance/MyGameInstance.h" // データ検索用
#include "Components/Image.h"

void UFishAlbumSlot::NativeConstruct()
{
    Super::NativeConstruct();
    if (Button_FishSelect)
    {
        Button_FishSelect->OnClicked.AddDynamic(this, &UFishAlbumSlot::InternalOnClicked);
    }
}

void UFishAlbumSlot::OnSetFishData(FString Name, int32 CaughtCount, float Record,float InMin,float InMax, UTexture2D* Icon)
{
    //メンバ関数
    MyFishName = Name;
    CurrentCaughtCount =CaughtCount;
    CurrentMaxSize = MaxSize;
    CurrentIcon = Icon;

    MinSize = InMin;
    MaxSize = InMax;
    // 1. 釣ったことがあるか判定
    bool bIsDiscovered = (CaughtCount > 0);

    // 2. 名前の表示設定
    if (Text_FishName)
    {
        // 釣ったことがあれば本名、なければ「？？？」
        FString DisplayName = bIsDiscovered ? Name : TEXT("???");
        Text_FishName->SetText(FText::FromString(DisplayName));
    }

    // 3. 釣った回数の表示
    if (Text_CatchCount)
    {
        Text_CatchCount->SetText(FText::FromString(FString::Printf(TEXT("x %d"), CaughtCount)));
    }

    // 4. 最大サイズの表示
    if (Text_MaxSize)
    {
        // 未発見ならサイズも伏せる
        FString SizeString = (CaughtCount > 0) ? FString::Printf(TEXT("%.1f cm"), MaxSize) : TEXT("--- cm");
        Text_MaxSize->SetText(FText::FromString(SizeString));
    }

    // 5. アイコンの表示設定
    if (Image_FishIcon && Icon)
    {
        Image_FishIcon->SetBrushFromTexture(Icon);

        // 釣っていない場合は「真っ黒（シルエット）」にする
        if (bIsDiscovered)
        {
            // 通常表示（白 = 元のテクスチャの色）
            Image_FishIcon->SetColorAndOpacity(FLinearColor::White);
        }
        else
        {
            // シルエット表示（黒）
            Image_FishIcon->SetColorAndOpacity(FLinearColor::Black);
        }
    }
}

void UFishAlbumSlot::InternalOnClicked()
{
    if (CurrentCaughtCount <= 0)    return;
    
    // 自分のプレイヤーのHUDを取得
    AFishingHUD* HUD = Cast<AFishingHUD>(GetOwningPlayer()->GetHUD());

    if (HUD)
    {
        // HUDに「この魚の詳細を出して！」と丸投げする
        HUD->ShowFishDetail(MyFishName, CurrentCaughtCount, CurrentMaxSize,MinSize,MaxSize,CurrentIcon);
    }
}