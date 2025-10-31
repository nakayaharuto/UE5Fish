#include "FishingRodActor.h"
#include "TimerManager.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

AFishingRodActor::AFishingRodActor()
{
    PrimaryActorTick.bCanEverTick = true;

    RodMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RodMesh"));
    RootComponent = RodMesh;
}

void AFishingRodActor::BeginPlay()
{
    Super::BeginPlay();

    CurrentState = EFishingState::Idle;

    // 🎯 ターゲットマーク生成（非表示）
    if (TargetMarkEffect)
    {
        TargetMarkComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            TargetMarkEffect,
            GetActorLocation() + GetActorForwardVector() * 300.f,
            FRotator::ZeroRotator
        );
        if (TargetMarkComponent)
        {
            TargetMarkComponent->SetVisibility(false);
        }
    }
}

void AFishingRodActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (CurrentState == EFishingState::Hooked || CurrentState == EFishingState::Reeling)
    {
        UpdateReeling(DeltaTime);
    }

    if (CurrentState == EFishingState::Casting)
    {
        CastPower = FMath::Min(CastPower + DeltaTime * 30.f, 100.f);
    }
}

void AFishingRodActor::ShowTargetMark(bool bShow)
{
    if (TargetMarkComponent)
        TargetMarkComponent->SetVisibility(bShow);
}

void AFishingRodActor::StartCasting()
{
    CurrentState = EFishingState::Casting;
    CastPower = 0.f;
    UE_LOG(LogTemp, Log, TEXT("🎯 キャスト溜め開始"));
}

void AFishingRodActor::ReleaseCasting()
{
    if (CurrentState != EFishingState::Casting) return;

    CurrentState = EFishingState::Waiting;
    UE_LOG(LogTemp, Log, TEXT("🎣 キャスト完了！パワー: %f"), CastPower);

    // 💥 糸エフェクト
    if (CastLineEffect)
    {
        FVector StartPos = GetActorLocation() + GetActorForwardVector() * 100.f;
        ActiveCastLine = UNiagaraFunctionLibrary::SpawnSystemAtLocation(
            GetWorld(),
            CastLineEffect,
            StartPos,
            GetActorRotation()
        );
    }

    // 🌊 着水エフェクト
    if (SplashEffect)
    {
        FVector WaterPos = GetActorLocation() + GetActorForwardVector() * (CastPower * 10.f);
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), SplashEffect, WaterPos);
    }

    // 🎯 マーク非表示
    ShowTargetMark(false);

    // 魚がかかるまでの遅延
    float BiteDelay = FMath::FRandRange(2.f, 5.f);
    GetWorldTimerManager().SetTimer(BiteTimerHandle, this, &AFishingRodActor::FishBite, BiteDelay, false);
}

void AFishingRodActor::FishBite()
{
    if (CurrentState != EFishingState::Waiting) return;

    CurrentState = EFishingState::Hooked;
    bFishOn = true;
    LineTension = 40.f;
    FishForce = 50.f;

    UE_LOG(LogTemp, Warning, TEXT("🐟 魚がヒット！"));
}

void AFishingRodActor::StartReel()
{
    if (CurrentState == EFishingState::Hooked)
    {
        CurrentState = EFishingState::Reeling;
        UE_LOG(LogTemp, Log, TEXT("🎞 巻き取り開始"));
    }
}

void AFishingRodActor::StopReel()
{
    if (CurrentState == EFishingState::Reeling)
    {
        CurrentState = EFishingState::Hooked;
        UE_LOG(LogTemp, Log, TEXT("🛑 巻き取り停止"));
    }
}

void AFishingRodActor::UpdateReeling(float DeltaTime)
{
    if (!bFishOn) return;

    float FishPull = FMath::Sin(GetWorld()->TimeSeconds * 2.f) * 15.f;
    FishForce = FMath::Clamp(50.f + FishPull, 20.f, 80.f);
    ReelSpeed = FMath::FInterpTo(ReelSpeed, 30.f, DeltaTime, 2.f);
    LineTension += ((FishForce - ReelSpeed) * 0.5f) * DeltaTime;
    LineTension = FMath::Clamp(LineTension, 0.f, 100.f);

    static float StableTime = 0.f;

    if (LineTension > 95.f)
    {
        UE_LOG(LogTemp, Error, TEXT("💥 糸が切れた！"));
        CurrentState = EFishingState::Fail;
        ResetFishing();
    }
    else if (LineTension < 10.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("❌ 魚が逃げた"));
        CurrentState = EFishingState::Fail;
        ResetFishing();
    }
    else if (LineTension > 40.f && LineTension < 60.f)
    {
        StableTime += DeltaTime;
        if (StableTime > 3.f)
        {
            UE_LOG(LogTemp, Log, TEXT("✅ 魚を釣り上げた！成功！"));
            CurrentState = EFishingState::Success;
            ResetFishing();
            StableTime = 0.f;
        }
    }
}

void AFishingRodActor::ResetFishing()
{
    GetWorldTimerManager().ClearTimer(BiteTimerHandle);
    CastPower = 0.f;
    LineTension = 0.f;
    FishForce = 0.f;
    ReelSpeed = 0.f;
    bFishOn = false;
    CurrentState = EFishingState::Idle;
}
