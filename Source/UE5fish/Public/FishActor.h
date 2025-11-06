#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishActor.generated.h"

UCLASS()
class UE5FISH_API AFishActor : public AActor
{
    GENERATED_BODY()

public:
    AFishActor();

    virtual void Tick(float DeletaTime) override;

    void StartFight(AActor* TargetLure);//“¦‚°Žn‚ß
    void StopFight();

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

private:
    bool bIsFighting = false;
    AActor* LureTraget = nullptr;
    float FightTimer = 0.f;
};
