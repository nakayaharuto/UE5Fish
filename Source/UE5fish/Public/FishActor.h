#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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

    //この魚の抵抗力の基底値
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish Stats")
    float BaseResistance = 5.0f;

    //抵抗力の最大増加係数（元気なときほど抵抗が増える度合い）
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fish Stats")
    float MaxResistanceMultiplier = 40.0f;

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
    int32 FishRarity; // レア度 (1:コモン, 5:レジェンドなど)

    void ShowFish();
    void HideFish();

    void SetFishData(const FString& Name, float Size, int32 Rarity, class UStaticMesh* FishMesh, float PlayerGaugeDecay);

protected:
    virtual void BeginPlay() override;

    

private:
    FTimerHandle HideTimerHandle;
};
