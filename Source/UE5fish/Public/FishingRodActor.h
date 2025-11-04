#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CableComponent.h"
#include "FishingRodActor.generated.h"

class ALureActor;
class AFishActor;
class UNiagaraSystem;
class UFishingWidget;

UCLASS()
class UE5FISH_API AFishingRodActor : public AActor
{
    GENERATED_BODY()
public:
    AFishingRodActor();

    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;

    void ShowCastTarget(const FVector& Location);
    void CastToLocation(const FVector& InTargetLocation);
    void ResetLure();
    void SpawnCaughtFish();

    UFUNCTION()
    void StartReel();

    UFUNCTION()
    void StopReel();


protected:
    UPROPERTY(VisibleAnywhere)
    USkeletalMeshComponent* RodMesh;

    UPROPERTY(EditDefaultsOnly, Category = "Fishing")
    TSubclassOf<ALureActor> LureClass;

    UPROPERTY()
    ALureActor* CurrentLure;

    UPROPERTY()
    AFishActor* CaughtFish;

    UPROPERTY(EditDefaultsOnly, Category = "Fishing")
    TSubclassOf<AFishActor> FishClass;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fishing")
    UCableComponent* LineCable;

    bool bIsCasting = false;
    bool bIsReeling = false;
    bool bIsFishBiting = false;
    bool bFishCaught = false;
    float FishReelProgress = 0.f;
    float ReelRequired = 1.f;
};
