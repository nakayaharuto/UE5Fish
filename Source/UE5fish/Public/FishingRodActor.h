#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishingRodActor.generated.h"

class UCableComponent;
class ALureActor;
class AFishActor;
class UNiagaraSystem;
class UUserWidget;
class UFishingWidget;

class AFishActor;

UCLASS()
class UE5FISH_API AFishingRodActor : public AActor
{
    GENERATED_BODY()

public:
    AFishingRodActor();

    virtual void Tick(float DeltaTime) override;

    void ShowCastTarget(const FVector& Location);
    void CastToLocation(const FVector& TargetLocation);
    void StartReel();
    void StopReel();

protected:
    virtual void BeginPlay() override;

    void SpawnCaughtFish();

    // --- components
    UPROPERTY(VisibleAnywhere)
    USkeletalMeshComponent* RodMesh;

    UPROPERTY(VisibleAnywhere)
    UCableComponent* Cable;

    // --- asset links (set in editor)
    UPROPERTY(EditDefaultsOnly, Category = "Fishing")
    TSubclassOf<ALureActor> LureClass;

    UPROPERTY(EditDefaultsOnly, Category = "Fishing")
    TSubclassOf<AFishActor> FishClass;

    UPROPERTY(EditDefaultsOnly, Category = "FX")
    UNiagaraSystem* CastMarkerFX;

    UPROPERTY(EditDefaultsOnly, Category = "FX")
    UNiagaraSystem* FishSplashFX;

    UPROPERTY(EditDefaultsOnly, Category = "UI")
    TSubclassOf<UUserWidget> FishingWidgetClass;

    // runtime
    UPROPERTY()
    ALureActor* CurrentLure;

    UPROPERTY()
    AFishActor* CaughtFish;

    UPROPERTY()
    UFishingWidget* FishingWidget;

    FVector TargetLocation;
    bool bIsCasting = false;
    bool bIsReeling = false;
    bool bIsFishBiting = false;
    bool bFishCaught = false;
    float FishReelProgress = 0.f;
    float ReelRequired = 1.0f; // 1.0 で釣り上げ成功
};

