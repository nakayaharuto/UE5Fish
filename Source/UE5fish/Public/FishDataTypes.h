// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine/DataTable.h"
#include "FishDataTypes.generated.h"

// 釣れる魚の種類ごとのデータ構造体
USTRUCT(BlueprintType)
struct FFishData : public FTableRowBase
{
    GENERATED_BODY()

public:
    // 魚の種類名 (例: "アジ", "マグロ")
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
    FString Name;

    // 魚のレア度 (例: 1=コモン, 5=レジェンド)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
    int32 Rarity;

    // 最小サイズ (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MinSize;

    // 最大サイズ (cm)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stats")
    float MaxSize;

    // 表示する魚のモデルアセット (SkeletalMeshまたはStaticMesh)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Assets")
    TSoftObjectPtr<class USkeletalMesh> FishMeshAsset;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish")
    float PlayerGaugeDecayContribution; // ★ この行を追加 ★
    // (その他、魚の習性やAIに必要なパラメータを追加可能)
};
