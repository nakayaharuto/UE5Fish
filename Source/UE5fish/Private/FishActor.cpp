#include "FishActor.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

AFishActor::AFishActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetLifeSpan(6.f); // 6秒で自動消去（必要なければ削除）
}

void AFishActor::BeginPlay()
{
    Super::BeginPlay();

}


void AFishActor::ShowFish()
{
    Mesh->SetVisibility(true);

    //3秒後非表示へ
    GetWorldTimerManager().SetTimer(HideTimerHandle, this, &AFishActor::HideFish, 3.f, false);
}

float AFishActor::GetCurrentDynamicResistance(float CurrentFishGauge, float GaugeMax) const
{
    // 抵抗力 = 基本抵抗 + ゲージ割合 * 最大増加係数
    float DynamicResistance = BaseResistance + (CurrentFishGauge / GaugeMax) * MaxResistanceMultiplier;
    return DynamicResistance;
}

void AFishActor::HideFish()
{
    Mesh->SetVisibility(false);
}


void AFishActor::Tick(float DeletaTime)
{
    Super::Tick(DeletaTime);

}

void AFishActor::SetFishData(const FString& Name, float Size, int32 Rarity, class UStaticMesh* FishMesh, float PlayerGaugeDecay)
{
    FishName = Name;
    SizeCm = Size;
    Rarity = Rarity;

    UE_LOG(LogTemp, Log, TEXT("AFishActor Data Set: Name=%s, Size=%.1f cm, Rarity=%d"), *Name, Size, Rarity);

    // Mesh は AFishActor.h で UStaticMeshComponent* Mesh; として定義されている前提
    if (Mesh && FishMesh)
    {
        // 外部から渡された UStaticMesh をコンポーネントに設定
        Mesh->SetStaticMesh(FishMesh);

        // サイズに応じたスケール調整
        // 例: 50cm を基準スケール 1.0 とした場合
        const float BaseSizeCm = 50.0f;

        // 実際のサイズと基準サイズの比率をスケールファクターとする
        float ScaleFactor = Size / BaseSizeCm;

        // 極端に大きくなりすぎないよう、スケールに制限を設けても良い
        ScaleFactor = FMath::Clamp(ScaleFactor, 0.5f, 5.0f);

        // 3Dモデルをスケーリング 
        Mesh->SetRelativeScale3D(FVector(ScaleFactor));
    }
    else
    {
        if (!Mesh)
        {
            UE_LOG(LogTemp, Error, TEXT("AFishActor: Mesh component is NULL!"));
        }
        if (!FishMesh)
        {
            UE_LOG(LogTemp, Error, TEXT("AFishActor: FishMesh asset is NULL!"));
        }
    }
}