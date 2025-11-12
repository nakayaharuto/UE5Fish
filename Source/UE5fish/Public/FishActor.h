#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishActor.generated.h"

UCLASS()
class UE5FISH_API AFishActor : public AActor
{
    GENERATED_BODY()

    //ãõÇÃÉÅÉbÉVÉÖ
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

public:
    AFishActor();

    virtual void Tick(float DeletaTime) override;

    void ShowFish();
    void HideFish();

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle HideTimerHandle;
};
