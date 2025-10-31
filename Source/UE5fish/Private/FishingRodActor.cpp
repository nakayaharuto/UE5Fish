#include "FishingRodActor.h"
#include "LureActor.h"
#include "FishActor.h"
#include "FishingWidget.h"
#include "CableComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "Components/SkeletalMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Blueprint/UserWidget.h"

AFishingRodActor::AFishingRodActor()
{
    PrimaryActorTick.bCanEverTick = true;

    RodMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("RodMesh"));
    RootComponent = RodMesh;

    Cable = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
    Cable->SetupAttachment(RodMesh, TEXT("RodTip")); // ソケット名 RodTip を想定
    Cable->SetComponentTickEnabled(false);  // Update停止
    Cable->bAttachEnd = true;
}

void AFishingRodActor::BeginPlay()
{
    Super::BeginPlay();

    if (RodMesh && RodMesh->SkeletalMesh)
    {
        FVector TipLocation = RodMesh->GetSocketLocation(TEXT("RodTip"));
        UE_LOG(LogTemp, Log, TEXT("RodTip: %s"), *TipLocation.ToString());
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("RodMesh or SkeletalMesh is null at BeginPlay!"));
    }

    if (FishingWidgetClass)
    {
        if (APlayerController* PC = UGameplayStatics::GetPlayerController(GetWorld(), 0))
        {
            FishingWidget = CreateWidget<UFishingWidget>(PC, FishingWidgetClass);
            if (FishingWidget)
            {
                FishingWidget->AddToViewport();
                FishingWidget->SetVisibility(ESlateVisibility::Hidden);
            }
        }
    }
}

void AFishingRodActor::ShowCastTarget(const FVector& Location)
{
    TargetLocation = Location;

    // キャストマーカーを表示（Niagara）
    if (CastMarkerFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), CastMarkerFX, Location, FRotator::ZeroRotator);
    }

    if (FishingWidget)
    {
        FishingWidget->SetCastMarkerLocation(Location);
        FishingWidget->SetVisibility(ESlateVisibility::Visible);
    }
}

void AFishingRodActor::CastToLocation(const FVector& InTargetLocation)
{
    if (bIsCasting || !LureClass) return;

    bIsCasting = true;
    bIsReeling = false;
    bIsFishBiting = false;
    bFishCaught = false;
    FishReelProgress = 0.f;

    if (FishingWidget)
    {
        FishingWidget->SetVisibility(ESlateVisibility::Hidden); // 狙い表示は投げたら消す
    }

    FVector StartLoc = RodMesh->GetSocketLocation(TEXT("RodTip"));
    FRotator Rot = (InTargetLocation - StartLoc).Rotation();

    FActorSpawnParameters Params;
    Params.Owner = this;

    CurrentLure = GetWorld()->SpawnActor<ALureActor>(LureClass, StartLoc, Rot, Params);
    if (!CurrentLure) { bIsCasting = false; return; }

    // Cable の終点をルアーに接続
    Cable->SetAttachEndTo(CurrentLure, NAME_None);
    Cable->SetVisibility(true);
    Cable->SetVisibility(true);

    // 飛ばす力（距離に応じて自動計算）
    float Distance = FVector::Dist(StartLoc, InTargetLocation);
    float Strength = FMath::Clamp(Distance * 4.f, 600.f, 3000.f);
    CurrentLure->AddImpulse((InTargetLocation - StartLoc).GetSafeNormal() * Strength);

    // 保存
    TargetLocation = InTargetLocation;
}

void AFishingRodActor::StartReel()
{
    if (!bIsCasting || !CurrentLure || bFishCaught) return;

    // リールは魚がヒットしていなくても巻ける（ゲーム性次第）
    bIsReeling = true;
    if (FishingWidget)
    {
        FishingWidget->ShowReelBar(true);
    }
}

void AFishingRodActor::StopReel()
{
    bIsReeling = false;
    if (FishingWidget)
    {
        FishingWidget->ShowReelBar(false);
    }
}

void AFishingRodActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (bIsCasting && CurrentLure)
    {
        // ランダムで魚がヒット（確率は調整可能）
        if (!bIsFishBiting)
        {
            // 発生率: 秒あたり 0.5% 〜 2% 程度の間で調整
            float HitChancePerSecond = 0.5f; // 0.5% / sec
            if (FMath::FRandRange(0.f, 100.f) < HitChancePerSecond * DeltaTime)
            {
                bIsFishBiting = true;
                if (FishingWidget) FishingWidget->ShowFishHit(true);
                // optional: 再生するサウンドやNiagaraをここで鳴らす
            }
        }

        // 糸を巻く処理（常に巻く場合は bIsReeling == true で）
        if (bIsReeling)
        {
            FVector Tip = RodMesh->GetSocketLocation(TEXT("RodTip"));
            FVector Pos = CurrentLure->GetActorLocation();
            FVector Dir = (Tip - Pos).GetSafeNormal();

            float ReelSpeed = 500.f; // 単位: cm/sec （調整）
            CurrentLure->SetActorLocation(Pos + Dir * ReelSpeed * DeltaTime, true);

            // 魚ヒット中なら釣り上げゲージを進める
            if (bIsFishBiting)
            {
                FishReelProgress += DeltaTime * 0.6f; // ゲージ増加速度
                if (FishingWidget) FishingWidget->SetReelProgress(FishReelProgress / ReelRequired);

                if (FishReelProgress >= ReelRequired)
                {
                    // 釣り上げ成功
                    bFishCaught = true;
                    bIsReeling = false;
                    bIsCasting = false;
                    SpawnCaughtFish();
                }
            }
        }
    }
}
void AFishingRodActor::SpawnCaughtFish()
{
    if (!FishClass) return;

    FVector Loc = RodMesh->GetSocketLocation(TEXT("RodTip"));
    FActorSpawnParameters Params;
    Params.Owner = this;

    CaughtFish = GetWorld()->SpawnActor<AFishActor>(FishClass, Loc, FRotator::ZeroRotator, Params);

    // スプラッシュFX
    if (FishSplashFX)
    {
        UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), FishSplashFX, Loc);
    }

    // 糸を非表示にしてルアー破棄
    if (CurrentLure)
    {
        CurrentLure->Destroy();
        CurrentLure = nullptr;
    }
    Cable->SetVisibility(false);

    // UI 通知
    if (FishingWidget)
    {
        FishingWidget->ShowFishHit(false);
        FishingWidget->OnFishCaught(CaughtFish);
        FishingWidget->SetCastMarkerLocation(TargetLocation);
    }
}
