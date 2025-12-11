// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/TextBlock.h" // UTextBlockを使用するために必要
#include "FishActor.h" // AFishActorへのポインタを使用する場合
#include "CaughtFish.generated.h"

class AFishActor;

UCLASS()
class UE5FISH_API UCaughtFish : public UUserWidget
{
	GENERATED_BODY()
	
	

public:
	UFUNCTION(BlueprintCallable, Category = "Fish Data")
	void SetFishData(AFishActor* CaughtFishActor);

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_FishName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Size;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Rarity;

protected:
	// ウィジェットが初期化されたときに実行される
	virtual bool Initialize() override;
	
	
};
