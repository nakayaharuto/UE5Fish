#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LureActor.generated.h"

UCLASS()
class UE5FISH_API ALureActor : public AActor
{
    GENERATED_BODY()

public:
    ALureActor();

    UFUNCTION()
    void AddImpulse(const FVector& Force);

protected:
    virtual void BeginPlay() override;

    UFUNCTION()
    void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
        UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

    /** メッシュ */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lure")
    class UStaticMeshComponent* Mesh;

    /** ケーブル接続ポイント */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Lure")
    class USceneComponent* AttachPoint;

    /** 水面などに当たったら停止する */
    bool bHasStopped = false;
};
