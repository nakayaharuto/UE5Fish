// Fill out your copyright notice in the Description page of Project Settings.


#include "FishingBattleWidget.h"

void UFishingBattleWidget::UpdateGauges(float PlayerPercent, float FishPercent)
{
	if(PlayerGaugeBar)
		PlayerGaugeBar->SetPercent(FMath::Clamp(PlayerPercent, 0.f, 1.f));

	if (FishGaugeBar)
		FishGaugeBar->SetPercent(FMath::Clamp(FishPercent, 0.f, 1.f));
}
