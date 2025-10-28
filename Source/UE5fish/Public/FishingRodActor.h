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

    /** ���b�V�� */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    USkeletalMeshComponent* RodMesh;

    /** �����|�����Ă��邩 */
    UPROPERTY(BlueprintReadWrite, Category = "Fishing")
    bool bFishBiting = false;

    /** �ނ�J�n */
    void StartFishing();
    void BeginChargeCast();
    void ReleaseCast();
    void ReelIn();
    void AdjustRodPitch(float Axis);


    /** ���E���� */
    void InputHorizontal(float Value);

    float CastCharge = 0.f;
    bool bIsCharging = false;
    bool bLineInWater = false;

    virtual void Tick(float DeltaTime) override;

    /** ���b�V���擾�p�֐� */
    FORCEINLINE USkeletalMeshComponent* GetMesh() const { return RodMesh; }
};
