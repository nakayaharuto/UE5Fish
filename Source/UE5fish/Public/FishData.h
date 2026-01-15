// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "FishData.generated.h"


// 釣れる魚の静的な情報を格納する構造体
USTRUCT(BlueprintType)
struct FFishAlbumData
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly)
	int32 TimesCaught = 0;

	UPROPERTY(BlueprintReadOnly)
	float MaxSize = 0.0f;

	UPROPERTY(BlueprintReadOnly)
	class UTexture2D* FishIcon = nullptr;
};

USTRUCT(BlueprintType)
struct FFishingFishData : public FTableRowBase
{
	GENERATED_BODY()

public:
	
	//魚の種類認識のID
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FName FishID;

	//魚の表示名
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FString FishName;

	//難易度や基礎地
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float BaseResistance;

	//釣りバトルでの最大抵抗力の増加係数
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float MaxResistanceMultiplier;

	//UIで表示するための画像（テクスチャ）
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	class UTexture2D* UITexture;
	// 最小サイズ (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MinSize;

	// 最大サイズ (cm)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
	float MaxSize;

	//希少度 (レアリティ)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	int32 Rarity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	float PlayerGaugeDecayContribution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
	FText FishDescription;
};
