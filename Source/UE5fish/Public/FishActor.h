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

protected:
    virtual void BeginPlay() override;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;
};
