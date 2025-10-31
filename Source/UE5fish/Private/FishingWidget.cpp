// Fill out your copyright notice in the Description page of Project Settings.


#include "FishingWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/CanvasPanelSlot.h"

void UFishingWidget::SetCastMarkerLocation(const FVector& WorldLocation)
{
    // CastMarkerImage は BP上の Image ウィジェットを想定
    if (!CastMarkerImage) return;

    APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0);
    if (!PC) return;

    FVector2D ScreenPos;
    bool bProjected = PC->ProjectWorldLocationToScreen(WorldLocation, ScreenPos);

    if (bProjected)
    {
        float ViewportScale = UWidgetLayoutLibrary::GetViewportScale(this);
        ScreenPos /= static_cast<double>(ViewportScale); // ← UE5.3以降対応

        if (UCanvasPanelSlot* CanvasSlot = Cast<UCanvasPanelSlot>(CastMarkerImage->Slot))
        {
            CanvasSlot->SetPosition(ScreenPos);
        }
    }
}

void UFishingWidget::ShowReelBar(bool bShow)
{
    if (ReelProgressBar)
    {
        ReelProgressBar->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
        if (!bShow) ReelProgressBar->SetPercent(0.f);
    }
}

void UFishingWidget::SetReelProgress(float Normalized)
{
    if (ReelProgressBar) ReelProgressBar->SetPercent(FMath::Clamp(Normalized, 0.f, 1.f));
}

void UFishingWidget::ShowFishHit(bool bShow)
{
    if (HitText) HitText->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Hidden);
}

void UFishingWidget::OnFishCaught(AFishActor* Fish)
{
    // BPで演出（魚モデルをUIに表示する等）を行うと良い
    ShowFishHit(false);
    ShowReelBar(false);
}
