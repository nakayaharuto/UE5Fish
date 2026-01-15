#include "FishActor.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

AFishActor::AFishActor()
{
    PrimaryActorTick.bCanEverTick = false;

    //Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    //RootComponent = Mesh;
    //Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AFishActor::BeginPlay()
{
    Super::BeginPlay();

}


void AFishActor::ShowFish()
{
  
}

float AFishActor::GetCurrentDynamicResistance(float CurrentFishGauge, float GaugeMax) const
{
    // 魚ゲージの進行度 (0.0 から 1.0)
    float ProgressRatio = CurrentFishGauge / GaugeMax;

    // 抵抗力は、魚ゲージが低いほど高くなる（元気なほど抵抗する）
    // 進行度が高くなると、抵抗力は Max から Base に向かって減少する。
    float ResistanceRatio = 1.0f - ProgressRatio; // 0% で 1.0, 100% で 0.0

    // 抵抗力の変動幅
    float ResistanceRange = MaxResistanceMultiplier - BaseResistance;

    // 現在の動的抵抗力 = ベース抵抗力 + (変動幅 * 抵抗比率)
    // FishGaugeが低いほど (ResistanceRatioが大きいほど) 抵抗力が MaxResistanceMultiplier に近づく
    float DynamicResistance = BaseResistance + (ResistanceRange * ResistanceRatio);

    // 抵抗力は常に正の値であることを保証
    return FMath::Max(DynamicResistance, 0.1f);
}

void AFishActor::HideFish()
{
    //Mesh->SetVisibility(false);
}


void AFishActor::Tick(float DeletaTime)
{
    Super::Tick(DeletaTime);

}

void AFishActor::SetFishData(const FString& Name, float Size, float PlayerGaugeDecay, float InBaseResistance,
    float InMaxResistanceMultiplier, UTexture2D* InTexture, FText InDescripion)
{

    FishName = Name;
    SizeCm = Size;
    this->PlayerGaugeDecayContribution = PlayerGaugeDecay; // これを追加！
    this->BaseResistance = InBaseResistance;               // これを追加！
    this->MaxResistanceMultiplier = InMaxResistanceMultiplier; // これを追加！
    this->UITexture = InTexture; // UI用の画像をここで受け取る
    this->FishDescription = InDescripion;
}