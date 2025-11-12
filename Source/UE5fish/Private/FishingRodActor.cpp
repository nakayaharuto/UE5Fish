#include "FishingRodActor.h"
#include "LureActor.h"
#include "FishActor.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"

AFishingRodActor::AFishingRodActor()
{
    PrimaryActorTick.bCanEverTick = true;
    RodMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RodMesh"));
    RootComponent = RodMesh;

    // ケーブル作成
    LineCable = CreateDefaultSubobject<UCableComponent>(TEXT("LineCable"));
    LineCable->SetupAttachment(RodMesh, TEXT("RodTip")); // ソケット RodTip からスタート
    LineCable->bAttachEnd = true; // 後でルアーに付ける
    LineCable->EndLocation = FVector::ZeroVector;
    LineCable->CableLength = 100.f; // 初期長さ
    LineCable->NumSegments = 20;
    LineCable->SubstepTime = 0.02f;
    LineCable->SolverIterations = 8;
    LineCable->CableWidth = 2.f;
    LineCable->bEnableCollision = false;
    LineCable->SetVisibility(false); // 初期は非表示
}

void AFishingRodActor::BeginPlay()
{
    Super::BeginPlay();
}

void AFishingRodActor::ResetRodState()
{
    // ルアーを安全にリセット（Destroyする前にケーブルの参照を解除）
    ResetLure();

    // 内部状態リセット
    bIsCasting = false;
    bIsReeling = false;
    bIsCharging = false;
    bIsFishBiting = false;
    bFishCaught = false;
    CastCharge = 0.f;
    FishReelProgress = 0.f;

    // 糸リセット
    if (LineCable)
    {
        LineCable->SetAttachEndTo(nullptr, NAME_None);
        LineCable->CableLength = 30.f;
        LineCable->SetVisibility(false);
    }

    // 竿を表示状態に（必要であれば）
    SetActorHiddenInGame(false);
}

void AFishingRodActor::CastToLocation(const FVector& InTargetLocation)
{
    if (bIsCasting || !LureClass) return;

    bIsCasting = true;
    bIsReeling = false;
    bIsFishBiting = false;
    bFishCaught = false;
    FishReelProgress = 0.f;

    FVector StartLoc = RodMesh->GetSocketLocation(TEXT("RodTip"));
    FRotator Rot = (InTargetLocation - StartLoc).Rotation();

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // ルアー生成
    CurrentLure = GetWorld()->SpawnActor<ALureActor>(LureClass, StartLoc, Rot, Params);
    if (CurrentLure)
    {
        CurrentLure->SetActorHiddenInGame(false);
        CurrentLure->LaunchLure(InTargetLocation, CastSpeed);

        // デリゲート登録
        if (!CurrentLure->OnHitWater.IsBound())
            CurrentLure->OnHitWater.AddDynamic(this, &AFishingRodActor::StopReel);

        if (!CurrentLure->OnFishHit.IsBound())
            CurrentLure->OnFishHit.AddDynamic(this, &AFishingRodActor::StartReel);

        // 糸接続
        if (LineCable)
        {
            LineCable->SetAttachEndTo(nullptr, NAME_None);
            LineCable->CableLength = FVector::Distance(StartLoc, InTargetLocation);
            LineCable->SetVisibility(true);
        }
        StartFishBastTimer();
    }
}

void AFishingRodActor::StartReel()
{
    if (!CurrentLure || bFishCaught) return;

    bIsReeling = true;

    FVector RodTip = RodMesh->GetSocketLocation(TEXT("RodTip"));
    CurrentLure->SetBeingReeled(true, RodTip);
}

void AFishingRodActor::StopReel()
{
    if (!CurrentLure || bFishCaught) return;

    bIsReeling = true;

    FVector RodTip = RodMesh->GetSocketLocation(TEXT("RodTip"));
    CurrentLure->SetBeingReeled(false, RodTip);
}

void AFishingRodActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!RodMesh) return; // 念のため安全ガード
    if (!LineCable) return; // Cableが無効なら以降スキップ
    FVector RodTip = RodMesh->GetSocketLocation(TEXT("RodTip"));

    // ルアーが存在しないときはCableの終端も外す
    if (!CurrentLure)
    {
        LineCable->SetAttachEndTo(nullptr, NAME_None);
        return;
    }

    FVector LurePos = CurrentLure->GetActorLocation();
    float Distance = FVector::Distance(RodTip, LurePos);

    // --- 🧭 リール処理 ---
    if (bIsReeling)
    {
        if (Distance > 5.f) // 近づきすぎで跳ねないように閾値を低く
        {
            float ReelSpeed = FMath::Clamp(Distance * 2.f, 300.f, 1200.f);
            FVector NewPos = FMath::VInterpTo(LurePos, RodTip, DeltaTime, ReelSpeed / FMath::Max(Distance, 1.f));
            CurrentLure->SetActorLocation(NewPos);
        }
        else
        {
            // 竿先に到達 → 正確に位置を合わせて停止
            CurrentLure->SetActorLocation(RodTip);
            bIsReeling = false;

            if (bIsFishBiting && CaughtFish)
            {
                OnFishCaught(); // 🎣 魚ヒット時のイベント
            }
        }
    }

    // --- 🪢 ケーブル長さの追従 ---
    {
        float TargetLength = FVector::Distance(RodTip, CurrentLure->GetActorLocation());

        // CableComponent が存在する限り滑らかに距離を合わせる
        LineCable->CableLength = FMath::FInterpTo(LineCable->CableLength, TargetLength, DeltaTime, 10.f);

        // Cable の終端が Lure に正しく追従していないときは再設定
        if (LineCable->GetAttachParentActor() != CurrentLure)
        {
            LineCable->SetAttachEndTo(CurrentLure, NAME_None);
        }
    }
}

void AFishingRodActor::ResetLure()
{
    if (CurrentLure)
    {
        // ケーブルの終端参照を先に外す（安全）
        if (LineCable)
            LineCable->SetAttachEndTo(nullptr, NAME_None);

        // 破棄してポインタをクリア
        CurrentLure->Destroy();
        CurrentLure = nullptr;
    }
}

void AFishingRodActor::SpawnCaughtFish()
{
    if (!FishClass) return;

    FVector Loc = RodMesh->GetSocketLocation(TEXT("RodTip"));
    FActorSpawnParameters Params;
    Params.Owner = this;

    CaughtFish = GetWorld()->SpawnActor<AFishActor>(FishClass, Loc, FRotator::ZeroRotator, Params);

    // ルアー処理とケーブル安全リセット
    ResetLure();
    if (LineCable)
    {
        LineCable->SetAttachEndTo(nullptr, NAME_None);
        LineCable->CableLength = 10.f;
        LineCable->SetVisibility(false);
    }
}

void AFishingRodActor::StartFishBastTimer()
{
    // 3〜8秒のランダムで魚がかかる
    float Delay = FMath::FRandRange(3.f, 8.f);
    GetWorldTimerManager().SetTimer(FishBiteTimerHandle, this, &AFishingRodActor::OnFishBite, Delay, false);
}

void AFishingRodActor::OnFishBite()
{
    if (!CurrentLure || bFishCaught) return;

    bIsFishBiting = true;

    //テキスト通知（ログ代わりに一旦）
    GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, TEXT("🎣 Hit!"));

    //魚を生成してルアーに紐づける
    FVector Loc = CurrentLure->GetActorLocation();
    FActorSpawnParameters Params;
    Params.Owner = this;

    CaughtFish = GetWorld()->SpawnActor<AFishActor>(FishClass, Loc, FRotator::ZeroRotator, Params);
    if (CaughtFish)
    {
        CaughtFish->AttachToActor(CurrentLure, FAttachmentTransformRules::KeepWorldTransform);
        CaughtFish->ShowFish();
    }
}

void AFishingRodActor::OnFishCaught()
{
    bFishCaught = true;
    bIsFishBiting = false;

    if (CaughtFish)
    {
        // 魚を一時的に竿の先に表示
        CaughtFish->DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
        FVector RodTip = RodMesh->GetSocketLocation(TEXT("RodTip"));
        CaughtFish->SetActorLocation(RodTip);
        CaughtFish->ShowFish();
    }

    // ルアー削除
    ResetLure();
}