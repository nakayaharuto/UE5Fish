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

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    void StartCasting();
    void ReleaseCasting();
    void FishBite();
    void StartReel();
    void StopReel();
    void UpdateReeling(float DeltaTime);
    void ResetFishing();

    // 🎯 エフェクト
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing|Effects")
    class UNiagaraSystem* TargetMarkEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing|Effects")
    class UNiagaraSystem* CastLineEffect;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing|Effects")
    class UNiagaraSystem* SplashEffect;

    UPROPERTY()
    class UNiagaraComponent* TargetMarkComponent;

    UPROPERTY()
    class UNiagaraComponent* ActiveCastLine;

    // Rod
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing")
    class USkeletalMeshComponent* RodMesh;

    // マーク表示関数
    UFUNCTION(BlueprintCallable, Category = "Fishing")
    void ShowTargetMark(bool bShow);

private:
    FTimerHandle BiteTimerHandle;
    float CastPower = 0.f;
    float LineTension = 0.f;
    float FishForce = 0.f;
    float ReelSpeed = 0.f;
    bool bFishOn = false;
    EFishingState CurrentState;
};

