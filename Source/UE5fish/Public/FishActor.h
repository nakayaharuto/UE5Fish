#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/SkeletalMeshComponent.h"
#include "FishActor.generated.h"

UCLASS()
class UE5FISH_API AFishActor : public AActor
{
    GENERATED_BODY()

    //魚のメッシュ
    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* Mesh;

    

  
public:
    AFishActor();

    virtual void Tick(float DeletaTime) override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fish Stats")
    float BaseResistance;

    //抵抗力の最大増加係数（元気なときほど抵抗が増える度合い）
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fish Stats")
    float MaxResistanceMultiplier;

    //プレイヤーゲージの減衰に寄与する係数 (魚の暴れやすさ)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish Stats")
    float PlayerGaugeDecayContribution = 0.5f;

    //抵抗力を計算し返す関数
    float GetCurrentDynamicResistance(float CurrentFishGauge, float GaugeMax) const;

    // 魚が持っているべきデータ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FishData")
    FString FishName;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FishData")
    float SizeCm; // 大きさ

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FishData")
    class UTexture2D* UITexture;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FishData")
    FText FishDescription;

    void ShowFish();
    void HideFish();

    void SetFishData(const FString& Name, float Size, float PlayerGaugeDecay, float InBaseResistance,
        float InMaxResistanceMultiplier, UTexture2D* InTexture, FText InDescripion);

protected:
    virtual void BeginPlay() override;

private:
    FTimerHandle HideTimerHandle;

   
};
