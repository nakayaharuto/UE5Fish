#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LureActor.generated.h"

class AMyCharacter;

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

    void UpdateFishingLine(const FVector& Start, const FVector& End);
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    // “Š‚°‚½î•ñ
    FVector StartLocation;
    FVector LaunchDirection;
    float LaunchSpeed;
    float LaunchTime = 0.f;  // “Š‚°‚Ä‚©‚ç‚ÌŒo‰ßŠÔ
    bool bIsLaunched = false;
    FTimerHandle AirResistTimer;

    // ƒŠ[ƒ‹’†
    bool bIsBeingReeled = false;
    bool bAirResistanceActive = false;
    FVector ReelTarget;

    // ’ïRŠÖŒW
    UPROPERTY(EditAnywhere, Category = "Physics")
    float AirResistance = 0.05f;//“Š‚°n‚ß‚Ì’ïR

    UPROPERTY(EditAnywhere, Category = "Physics")
    float MaxDistance = 3000.f;//”ò‹——£‚ÌŒÀŠE’l

    UPROPERTY(EditAnywhere, Category = "Physics")
    float DistanceDampingFactor = 0.4f;//’ïR‚ÌŒÀŠE’l

    // ƒŠ[ƒ‹‚Ì—Í‚Ì‹­‚³
    UPROPERTY(EditAnywhere, Category = "Reeling")
    float ReelForce = 3000.f;

    UFUNCTION()
    void EnableAirResistance();
};