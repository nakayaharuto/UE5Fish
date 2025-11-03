// FishingCastMarkerActor.h
#pragma once
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishingCastMarkerActor.generated.h"

UCLASS()
class UE5FISH_API AFishingCastMarkerActor : public AActor
{
    GENERATED_BODY()
public:
    AFishingCastMarkerActor();

protected:
    UPROPERTY(VisibleAnywhere)
    class UNiagaraComponent* MarkerFX; // Ç‹ÇΩÇÕ StaticMeshComponent Ç≈Ç‡OK
};
