// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "CaughtFish.generated.h"

/**
 * 
 */
class AFishActor;

UCLASS()
class UE5FISH_API UCaughtFish : public UUserWidget
{
	GENERATED_BODY()
	
	

public:
	UFUNCTION(BlueprintCallable, Category = "Fish Data")
	void SetCaughtFishData(AFishActor* CaughtFishActor);


private:

	FText FishName;
	float FishSize;

	
	
};
