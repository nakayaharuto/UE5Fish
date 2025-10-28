#include "FishingRodActor.h"
#include "Components/SkeletalMeshComponent.h"

AFishingRodActor::AFishingRodActor()
{
    PrimaryActorTick.bCanEverTick = true;

    RodMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RodMesh"));
    RootComponent = RodMesh;
}

void AFishingRodActor::BeginChargeCast()
{
    bIsCharging = true;
    CastCharge = 0.f;
}

void AFishingRodActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsCharging)
    {
        CastCharge = FMath::Min(CastCharge + DeltaTime * 50.f, 100.f); // チャージ量増加
    }
}

void AFishingRodActor::ReelIn()
{
    if (bLineInWater)
    {
        UE_LOG(LogTemp, Log, TEXT("Reeling in..."));
        // 糸を巻き取る処理（Actorを近づけるなど）
    }
}

void AFishingRodActor::AdjustRodPitch(float Axis)
{
    FRotator NewRot = GetActorRotation();
    NewRot.Pitch = FMath::Clamp(NewRot.Pitch + Axis * 1.5f, -30.f, 45.f);
    SetActorRotation(NewRot);
}

void AFishingRodActor::ReleaseCast()
{
    if (!bIsCharging) return;
    bIsCharging = false;
    bLineInWater = true;

    // 投げアニメーション or パーティクル呼び出し
    UE_LOG(LogTemp, Log, TEXT("Casting line distance: %f"), CastCharge);

    // 距離に応じてルアーを飛ばすなど
}

void AFishingRodActor::StartFishing()
{
    UE_LOG(LogTemp, Log, TEXT("釣り開始"));
    bFishBiting = false;
}

void AFishingRodActor::InputHorizontal(float Value)
{
    if (Value != 0.f)
    {
        // 左右(Yaw)回転
        AddActorLocalRotation(FRotator(0.f, Value * 5.f, 0.f));
    }
}
