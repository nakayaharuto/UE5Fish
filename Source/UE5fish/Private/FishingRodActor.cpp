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
}

void AFishingRodActor::BeginPlay()
{
    Super::BeginPlay();
}

void AFishingRodActor::ShowCastTarget(const FVector& Location)
{
    // デバッグ用：ログ出力
    //UE_LOG(LogTemp, Log, TEXT("Cast Target: %s"), *Location.ToString());
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
    CurrentLure = GetWorld()->SpawnActor<ALureActor>(LureClass, StartLoc, Rot, Params);

    if (CurrentLure)
    {
        CurrentLure->OnHitWater.AddDynamic(this, &AFishingRodActor::StopReel); // 仮: 水に当たったら停止
        CurrentLure->OnFishHit.AddDynamic(this, &AFishingRodActor::StartReel); // 仮: 魚がヒットしたら自動リール
        CurrentLure->LaunchLure(InTargetLocation, 1500.f);
    }

    if (CurrentLure && LineCable)
    {
        // ルアーの位置にケーブルの終端を接続
        LineCable->SetAttachEndTo(CurrentLure, NAME_None);
    }
}

void AFishingRodActor::StartReel()
{
    if (!CurrentLure || bFishCaught) return;

    bIsReeling = true;
    CurrentLure->SetBeingReeled(true);
}

void AFishingRodActor::StopReel()
{
    bIsReeling = false;
    if (CurrentLure)
        CurrentLure->SetBeingReeled(false);
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

        // ケーブルの長さを更新
        LineCable->CableLength = FVector::Distance(RodTip, NewPos);

        // デバッグ出力
        UE_LOG(LogTemp, Log, TEXT("Reeling lure at position X=%f Y=%f Z=%f"),
            NewPos.X, NewPos.Y, NewPos.Z);
    }

    // ルアーが竿に近づいたら巻き取り完了
    if (CurrentLure)
    {
        FVector RodTip = RodMesh->GetSocketLocation(TEXT("RodTip"));
        if (FVector::Dist(CurrentLure->GetActorLocation(), RodTip) < 100.f)
        {
            bIsReeling = false;
            UE_LOG(LogTemp, Log, TEXT("Lure fully reeled in"));

            CurrentLure->Destroy();
            CurrentLure = nullptr;

            // ケーブルの終端をリセット
            if (LineCable)
            {
                LineCable->SetAttachEndTo(nullptr, NAME_None);
                LineCable->CableLength = 0.f;
                LineCable->SetVisibility(false);
            }
        }
    }
}

void AFishingRodActor::ResetLure()
{
    if (CurrentLure)
    {
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

    // 糸を非表示にしてルアー破棄
    if (CurrentLure)
    {
        CurrentLure->Destroy();
        CurrentLure = nullptr;
    }

    if (LineCable)
    {
        LineCable->SetAttachEndTo(CurrentLure, NAME_None);
        LineCable->CableLength = 0.f;
        LineCable->SetVisibility(true);
    }
}
