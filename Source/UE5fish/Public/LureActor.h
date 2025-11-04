#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LureActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnHitWaterDelegate);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnFishHitDelegate);

UCLASS()
class UE5FISH_API ALureActor : public AActor
{
    GENERATED_BODY()
public:
    ALureActor();

    virtual void Tick(float DeltaTime) override;
    void LaunchLure(const FVector& InTarget, float InSpeed);

    UFUNCTION(BlueprintCallable)
    void SetBeingReeled(bool bReeled) { bIsBeingReeled = bReeled; }

    UStaticMeshComponent* GetMesh() const { return Mesh; }

    UPROPERTY(BlueprintAssignable)
    FOnHitWaterDelegate OnHitWater;

    UPROPERTY(BlueprintAssignable)
    FOnFishHitDelegate OnFishHit;

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    FVector StartLocation;
    FVector TargetLocation;
    float Speed = 1000.f;
    float MaxDistance = 1500.f;

    bool bIsFlying = false;
    bool bIsBeingReeled = false;
    bool bHitWater = false;
    bool bFishHit = false;

    UPROPERTY(EditAnywhere, Category = "Fishing")
    TSubclassOf<AActor> WaterActorClass;

    UPROPERTY(EditAnywhere, Category = "Fishing")
    float FishHitChancePerSecond = 0.5f;
};