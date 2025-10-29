#include "FishingRodActor.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"

AFishingRodActor::AFishingRodActor()
{
    PrimaryActorTick.bCanEverTick = true;

    RodMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RodMesh"));
    RootComponent = RodMesh;
}

void AFishingRodActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsCharging)
        CastCharge = FMath::Min(CastCharge + DeltaTime * 50.f, 100.f);

    if (CurrentState == EFishingState::Reeling || CurrentState == EFishingState::Hooked)
        UpdateReeling(DeltaTime);
}

void AFishingRodActor::BeginChargeCast()
{
    if (CurrentState != EFishingState::Idle) return;
    bIsCharging = true;
    CastCharge = 0.f;
}

void AFishingRodActor::ReleaseCast()
{
    if (!bIsCharging) return;
    bIsCharging = false;
    bLineInWater = true;

    UE_LOG(LogTemp, Log, TEXT("Casting line distance: %f"), CastCharge);

    float BiteDelay = FMath::RandRange(3.f, 8.f);
    GetWorldTimerManager().SetTimer(FishBiteTimer, this, &AFishingRodActor::FishBite, BiteDelay, false);
}

void AFishingRodActor::StartFishing()
{
    float BiteDelay = FMath::FRandRange(2.f, 6.f);
    GetWorldTimerManager().SetTimer(BiteTimerHandle, this, &AFishingRodActor::FishBite, BiteDelay, false);
    CurrentState = EFishingState::Waiting;
    UE_LOG(LogTemp, Log, TEXT("🐠 待機中..."));
}

void AFishingRodActor::FishBite()
{
    if (CurrentState != EFishingState::Waiting) return;

    CurrentState = EFishingState::Hooked;
    UE_LOG(LogTemp, Warning, TEXT("🐟 魚がヒット！テンション開始"));
    LineTension = 40.f;
    FishForce = 50.f;
}

void AFishingRodActor::StartReel()
{
    if (CurrentState == EFishingState::Hooked || CurrentState == EFishingState::Reeling)
    {
        bIsReeling = true;
        CurrentState = EFishingState::Reeling;
    }
}

void AFishingRodActor::StopReel()
{
    bIsReeling = false;
}

void AFishingRodActor::UpdateReeling(float DeltaTime)
{
    float FishPull = FMath::Sin(GetWorld()->TimeSeconds * 1.8f) * 15.f;
    FishForce = FMath::Clamp(50.f + FishPull, 20.f, 80.f);

    ReelSpeed = bIsReeling ? FMath::FInterpTo(ReelSpeed, 30.f, DeltaTime, 2.f)
        : FMath::FInterpTo(ReelSpeed, 0.f, DeltaTime, 2.f);

    float AngleFactor = 1.f - (RodPitch / 45.f);
    LineTension += ((FishForce - ReelSpeed) * AngleFactor) * DeltaTime * 0.8f;
    LineTension = FMath::Clamp(LineTension, 0.f, 100.f);

    if (LineTension > 95.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("💥 糸が切れた！"));
        CurrentState = EFishingState::Fail;
        ResetFishingState();
    }
    else if (LineTension < 10.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ 緩みすぎて逃げられた"));
        CurrentState = EFishingState::Fail;
        ResetFishingState();
    }
    else if (bIsReeling && FMath::IsNearlyEqual(LineTension, 50.f, 10.f))
    {
        StableTime += DeltaTime;
        if (StableTime > 2.5f)
        {
            UE_LOG(LogTemp, Log, TEXT("🎯 魚を釣り上げた！成功！"));
            CurrentState = EFishingState::Success;
            ResetFishingState();
        }
    }
}

void AFishingRodActor::AdjustRodPitch(float Axis)
{
    if (CurrentState == EFishingState::Idle) return;
    RodPitch = FMath::Clamp(RodPitch + Axis * 2.f, -10.f, 45.f);
    FRotator NewRot = GetActorRotation();
    NewRot.Pitch = RodPitch;
    SetActorRotation(NewRot);
}

void AFishingRodActor::AdjustRodYaw(float Axis)
{
    if (CurrentState == EFishingState::Idle) return;
    RodYaw += Axis * 2.f;
    FRotator NewRot = GetActorRotation();
    NewRot.Yaw = RodYaw;
    SetActorRotation(NewRot);
}

void AFishingRodActor::AdjustTension(float Delta)
{
    LineTension = FMath::Clamp(LineTension + Delta, 0.f, 100.f);
}

void AFishingRodActor::AddFishProgress(float Delta)
{
    FishProgress = FMath::Clamp(FishProgress + Delta, 0.f, 100.f);
}

void AFishingRodActor::ResetFishingState()
{
    UE_LOG(LogTemp, Log, TEXT("🎣 フィッシングリセット"));

    GetWorldTimerManager().ClearTimer(BiteTimerHandle);
    GetWorldTimerManager().ClearTimer(FishBiteTimer);

    bIsCharging = false;
    bLineInWater = false;
    bIsReeling = false;
    CastCharge = 0.f;
    LineTension = 0.f;
    FishProgress = 0.f;
    FishForce = 0.f;
    ReelSpeed = 0.f;
    RodPitch = 10.f;
    RodYaw = 0.f;
    StableTime = 0.f;
}
