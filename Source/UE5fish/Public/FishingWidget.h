// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "FishingWidget.generated.h"

class UImage;
class UProgressBar;
class UTextBlock;
class AFishActor;

/**
 * 
 */
UCLASS()
class UE5FISH_API UFishingWidget : public UUserWidget
{
	GENERATED_BODY()
	
public:

    // CastMarker•\Ž¦—p
    UPROPERTY(meta = (BindWidget))
    UImage* CastMarkerImage;

    UFUNCTION(BlueprintCallable)
    void SetCastMarkerLocation(const FVector& WorldLocation);

    UFUNCTION(BlueprintCallable)
    void ShowReelBar(bool bShow);

    UFUNCTION(BlueprintCallable)
    void SetReelProgress(float Normalized);

    UFUNCTION(BlueprintCallable)
    void ShowFishHit(bool bShow);

    UFUNCTION(BlueprintCallable)
    void OnFishCaught(AFishActor* Fish);

protected:
    UPROPERTY(meta = (BindWidget))
    UProgressBar* ReelProgressBar;

    UPROPERTY(meta = (BindWidget))
    UTextBlock* HitText;
};
