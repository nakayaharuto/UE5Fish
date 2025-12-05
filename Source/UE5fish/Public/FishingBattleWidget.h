// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/ProgressBar.h"
#include "FishingBattleWidget.generated.h"

/**
 * 
 */
UCLASS()
class UE5FISH_API UFishingBattleWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	//ゲージ更新用関数
	UFUNCTION(BlueprintCallable)
	void UpdateGauges(float PlayerPercent, float FishPercent);

protected:
	//参照
	UPROPERTY(meta = (BindWiget))
	UProgressBar* PlayerGaugeBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* FishGaugeBar;
};
