// Fill out your copyright notice in the Description page of Project Settings.
#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishingRodActor.generated.h"

UCLASS()
class UE5FISH_API AFishingRodActor : public AActor
{
    GENERATED_BODY()

public:
    AFishingRodActor();

    /** メッシュ */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    USkeletalMeshComponent* RodMesh;

    /** 魚が掛かっているか */
    UPROPERTY(BlueprintReadWrite, Category = "Fishing")
    bool bFishBiting = false;

    /** 釣り開始 */
    void StartFishing();
    void BeginChargeCast();
    void ReleaseCast();
    void ReelIn();
    void AdjustRodPitch(float Axis);


    /** 左右操作 */
    void InputHorizontal(float Value);

    float CastCharge = 0.f;
    bool bIsCharging = false;
    bool bLineInWater = false;

    virtual void Tick(float DeltaTime) override;

    /** メッシュ取得用関数 */
    FORCEINLINE USkeletalMeshComponent* GetMesh() const { return RodMesh; }
};
