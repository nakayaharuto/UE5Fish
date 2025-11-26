#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LureActor.generated.h"

class AFishActor;
class UCableComponent;
class AMyCharacter;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLureEvent);

UCLASS()
class UE5FISH_API ALureActor : public AActor
{
    GENERATED_BODY()

public:
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing")
    TSubclassOf<AFishActor> FishClass;

    UPROPERTY(BlueprintAssignable)
    FLureEvent OnHitWater;

    UPROPERTY(BlueprintAssignable)
    FLureEvent OnFishHit;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    UPROPERTY(VisibleAnywhere)
    UCableComponent* Cable; // 糸の見た目


public:
    

    ALureActor();
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    // 既存 API を残す
    void LaunchLure(const FVector& InTarget, float InSpeed);
    void SetBeingReeled(bool bReeling, const FVector& ReelTarget);
    void NotifyActorBeginOverlap(AActor* OtherActor) override;
    void ResetLure();
    void TryFishHit();

    // 追加：単純な Cast 用（保持する場合に備えて）
    void CastLure(const FVector& Direction, float Power) { LaunchLure(GetActorLocation() + Direction * 10.f, Power); }
    void OnFishHitConfirmed();

    bool IsReeling() const { return bIsBeingReeled; }

protected:
    // 投げた情報
    FVector StartLocation;
    FVector LaunchDirection;
    float LaunchSpeed = 0.f;
    float LaunchTime = 0.f;  // 投げてからの経過時間
    bool bIsFishHit = false; // 魚がヒットしたか
    bool bIsLaunched = false;
    FTimerHandle AirResistTimer;

    // リール中
    bool bIsBeingReeled = false;
    bool bAirResistanceActive = false;
    FVector ReelTarget;
   
    // ランダムヒット用
    UPROPERTY()
    AFishActor* HitFish;  // 表示用魚
    FTimerHandle HitTimerHandle;

    // 設定可能パラメータ
    UPROPERTY(EditAnywhere, Category = "Physics")
    float AirResistance = 0.05f;

    UPROPERTY(EditAnywhere, Category = "Physics")
    float MaxCastDistance = 2500.f; // 一定距離で投げ挙動終了

    UPROPERTY(EditAnywhere, Category = "Physics")
    float MaxCastTime = 2.0f; // 投げてからの最大時間（時間経過でも停止）

    UPROPERTY(EditAnywhere, Category = "Reeling")
    float ReelSpeed = 350.f; // 一定速度で巻く（固定）

    UPROPERTY(EditAnywhere, Category = "Reeling")
    float ReelArrivalThreshold = 25.f; // 竿先に到達とみなす距離

    // ランダムヒット範囲 (秒)
    UPROPERTY(EditAnywhere, Category = "Fish")
    FVector2D HitDelayRange = FVector2D(1.2f, 4.0f);

    // 内部関数
    UFUNCTION()
    void EnableAirResistance();

    // 内部関数
    UFUNCTION()
    void SpawnHitFish();         // ランダムで魚を出す
    
    UPROPERTY()
    AFishActor* SpawnedFish = nullptr;

    void EndCast();              // 一定距離/timeで物理を止める
    void ReelStep(float DeltaTime); // 一定速度で巻く処理
};