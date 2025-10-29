#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishingRodActor.generated.h"

UENUM(BlueprintType)
enum class EFishingState : uint8
{
    Idle,
    Casting,
    Waiting,
    Hooked,
    Reeling,
    Success,
    Fail
};

UCLASS()
class UE5FISH_API AFishingRodActor : public AActor
{
    GENERATED_BODY()

public:
    AFishingRodActor();

    virtual void Tick(float DeltaTime) override;

    /** メッシュ */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Components")
    USkeletalMeshComponent* RodMesh;

    /** 現在の釣り状態 */
    UPROPERTY(BlueprintReadOnly, Category = "Fishing")
    EFishingState CurrentState = EFishingState::Idle;

    /** 糸テンション (0〜100) */
    UPROPERTY(BlueprintReadOnly, Category = "Fishing")
    float LineTension = 0.f;

    /** 魚の進行度 (0〜100) */
    UPROPERTY(BlueprintReadOnly, Category = "Fishing")
    float FishProgress = 0.f;

    /** 投げ距離チャージ */
    void BeginChargeCast();
    void ReleaseCast();

    /** リール操作 */
    UFUNCTION(BlueprintCallable, Category = "Fishing")
    void StartReel();

    UFUNCTION(BlueprintCallable, Category = "Fishing")
    void StopReel();

    /** ロッド角度調整 */
    void AdjustRodPitch(float Axis);
    void AdjustRodYaw(float Axis);

    /** フィッシング開始 */
    void StartFishing();

    /** 線テンション・進行度操作関数 */
    void AdjustTension(float Delta);
    void AddFishProgress(float Delta);

    /** リセット */
    void ResetFishingState();

private:
    bool bIsCharging = false;
    bool bLineInWater = false;
    bool bIsReeling = false;
    float CastCharge = 0.f;

    float RodPitch = 10.f;
    float RodYaw = 0.f;

    FTimerHandle BiteTimerHandle;
    FTimerHandle FishBiteTimer;

    float FishForce = 0.f;
    float ReelSpeed = 0.f;

    float StableTime = 0.f; // 釣り成功判定用

    void FishBite();
    void UpdateReeling(float DeltaTime);
};
