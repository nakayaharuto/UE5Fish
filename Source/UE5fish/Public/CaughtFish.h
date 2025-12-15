// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Image.h"    // ★ UImageクラスの定義 ★
#include "Components/Button.h"
#include "Components/TextBlock.h" // UTextBlockを使用するために必要
#include "FishActor.h" // AFishActorへのポインタを使用する場合
#include "CaughtFish.generated.h"

class UTextBlock;
class AFishActor;

UCLASS()
class UE5FISH_API UCaughtFish : public UUserWidget
{
	GENERATED_BODY()
	
	

public:
	// ウィジェットが初期化されたときに実行される
	virtual bool Initialize() override;

	UPROPERTY()
	AFishActor* CurrentFishActor; // 破棄対象の魚を保持

	UFUNCTION()
	void OnCloseButtonClicked();

	UFUNCTION(BlueprintCallable, Category = "Fish Data")
	void SetFishData(AFishActor* Fish);

public:
	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_FishName;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Size;

	UPROPERTY(meta = (BindWidget))
	class UTextBlock* Text_Rarity;

	UPROPERTY(meta = (BindWidget))
	class UImage* Image_FishIcon;

	UPROPERTY(meta = (BindWidget))
	class UButton* Button_Exit;

};
