#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LureActor.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FLureEvent);

UCLASS()
class UE5FISH_API ALureActor : public AActor
{
    GENERATED_BODY()

public:
    ALureActor();
    virtual void Tick(float DeltaTime) override;

    void LaunchLure(const FVector& InTarget, float InSpeed);
    void SetBeingReeled(bool bReeling, const FVector& ReelTarget);
    void ResetLure();

    UPROPERTY(BlueprintAssignable)
    FLureEvent OnHitWater;

    UPROPERTY(BlueprintAssignable)
    FLureEvent OnFishHit;

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    // “Š‚°‚½î•ñ
    FVector StartLocation;
    FVector LaunchDirection;
    float LaunchSpeed;
    float LaunchTime = 0.f;  // “Š‚°‚Ä‚©‚ç‚ÌŒo‰ßŠÔ
    bool bIsLaunched = false;

    // ƒŠ[ƒ‹’†
    bool bIsBeingReeled = false;
    FVector ReelTarget;

    // ’ïRŠÖŒW
    UPROPERTY(EditAnywhere, Category = "Physics")
    float AirResistance = 0.0001f;

    UPROPERTY(EditAnywhere, Category = "Physics")
    float MaxDistance = 800.f;

    UPROPERTY(EditAnywhere, Category = "Physics")
    float DistanceDampingFactor = 0.5f;

    // ƒŠ[ƒ‹‚Ì—Í‚Ì‹­‚³
    UPROPERTY(EditAnywhere, Category = "Reeling")
    float ReelForce = 3000.f;
};