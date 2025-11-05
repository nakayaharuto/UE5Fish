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
        LineCable->SetAttachEndTo(CurrentLure, NAME_None);
        LineCable->CableLength = FVector::Distance(StartLoc, InTargetLocation);
        LineCable->SetVisibility(true);
    }
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

    if (bIsReeling && CurrentLure && LineCable)
    {
        FVector RodTip = RodMesh->GetSocketLocation(TEXT("RodTip"));
        FVector LurePos = CurrentLure->GetActorLocation();

        // 距離に応じてリール速度を調整
        float Distance = FVector::Distance(RodTip, LurePos);
        float ReelSpeed = FMath::Clamp(Distance * 2.f, 300.f, 1200.f); // cm/sec

        // 滑らかに移動
        FVector NewPos = FMath::VInterpTo(LurePos, RodTip, DeltaTime, ReelSpeed / Distance);
        CurrentLure->SetActorLocation(NewPos);

        //糸の長さを自動同期
        float TargetLength = Distance * 1.05f;
        LineCable->CableLength = FMath::FInterpTo(LineCable->CableLength, TargetLength, DeltaTime, 3.f);

        if (Distance < 100.f)
        {
            bIsReeling = false;

            // 糸はそのまま保持、ルアーは削除しない！
            if (LineCable)
            {
                LineCable->SetAttachEndTo(CurrentLure, NAME_None);
                LineCable->CableLength = 30.f;
            }

            // 位置を固定（再利用できるように）
            FVector SnapPos = RodTip - (RodTip - LurePos).GetSafeNormal() * 10.f;
            CurrentLure->SetActorLocation(SnapPos);
        }
    }

    //ルアーが存在する間、糸の長さを常に追従させる（キャスト中も）
    if (CurrentLure && LineCable)
    {
        FVector RodTip = RodMesh->GetSocketLocation(TEXT("RodTip"));
        FVector LurePos = CurrentLure->GetActorLocation();
        float Distance = FVector::Distance(RodTip, LurePos);

        // 滑らかに長さを更新
        LineCable->CableLength = FMath::FInterpTo(LineCable->CableLength, Distance, DeltaTime, 5.f);
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
