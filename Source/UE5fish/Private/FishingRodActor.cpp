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

    // デフォルトゲージ値
    PlayerGauge = 0.f;
    FishGauge = 0.f;
    GaugeMax = 100.f;

    // 調整用デフォルト（必要なら調整）
    PlayerGaugeIncreasePerClick = 12.f;
    FishGaugeIncreasePerClick = 9.f;
    PlayerGaugeDecayRate = 8.f;
    FishGaugeDecayRate = 6.f;
}

void AFishingRodActor::BeginPlay()
{
    Super::BeginPlay();


    if (CurrentLure)
    {
        CurrentLure->OnFishHit.AddDynamic(this, &AFishingRodActor::OnFishHitEvent);
    }
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

    bIsFishBattle = false;
    PlayerGauge = 0.f;
    FishGauge = 0.f;

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

    FVector StartLoc = RodMesh->GetSocketLocation(TEXT("RodTip"));
    FRotator Rot = (InTargetLocation - StartLoc).Rotation();

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    CurrentLure = GetWorld()->SpawnActor<ALureActor>(LureClass, StartLoc, Rot, Params);
    if (CurrentLure)
    {
        CurrentLure->SetActorHiddenInGame(false);

        // LaunchLure に全ての投げ挙動を任せる
        CurrentLure->LaunchLure(InTargetLocation, CastSpeed);

        if (!CurrentLure->OnFishHit.IsBound())
            CurrentLure->OnFishHit.AddDynamic(this, &AFishingRodActor::StartReel);

        // 糸接続
        if (LineCable)
        {
            LineCable->SetAttachEndTo(CurrentLure, NAME_None); // ルアーのRootComponentに接続
            LineCable->CableLength = FVector::Distance(StartLoc, InTargetLocation);
            LineCable->SetVisibility(true);
        }
    }
}

void AFishingRodActor::StartReel()
{
    if (!CurrentLure || bFishCaught) return;
    if (IsValid(CurrentLure->HitFish))
    {
        // このタイミングで初めてルアーに追従させる
        CurrentLure->HitFish->AttachToActor(
            CurrentLure,
            FAttachmentTransformRules::KeepWorldTransform
        );

        UE_LOG(LogTemp, Warning, TEXT("Fish attached to lure because reel started!"));
    }

    bIsReeling = false;

    FVector RodTip = RodMesh->GetSocketLocation(TEXT("RodTip"));
    CurrentLure->SetBeingReeled(true, RodTip);
    
}

void AFishingRodActor::StopReel()
{
    if (!CurrentLure || bFishCaught) return;

    bIsReeling = false;

    FVector RodTip = RodMesh->GetSocketLocation(TEXT("RodTip"));
    CurrentLure->SetBeingReeled(false, RodTip);

    return;
}

void AFishingRodActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!RodMesh || !LineCable) return;
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
            if (IsValid(CurrentLure))
            {
                CurrentLure->SetActorLocation(NewPos);
            }
        }
        else
        {
            if (IsValid(CurrentLure))
            {
                CurrentLure->SetActorLocation(RodTip);
            }
            bIsReeling = false;

            if (bIsFishBiting && IsValid(CaughtFish))
            {
                OnFishCaught(); // 🎣 魚を釣り上げる処理
            }

            if (IsValid(CurrentLure))
            {
                CurrentLure->SetBeingReeled(false, RodTip);
            }
        }
    }

    // --- 🪢 ケーブル長さの追従 ---
    if (IsValid(LineCable) && IsValid(CurrentLure))
    {
        float TargetLength = FVector::Distance(RodTip, CurrentLure->GetActorLocation());
        LineCable->CableLength = FMath::FInterpTo(LineCable->CableLength, TargetLength, DeltaTime, 10.f);
    }

    // --- 🟩 バトル中のゲージ時間経過 ---
    if (bIsFishBattle)
    {
        // 減衰
        PlayerGauge -= PlayerGaugeDecayRate * DeltaTime;
        FishGauge -= FishGaugeDecayRate * DeltaTime;

        PlayerGauge = FMath::Clamp(PlayerGauge, 0.f, GaugeMax);
        FishGauge = FMath::Clamp(FishGauge, 0.f, GaugeMax);

        CheckFishBattleState();
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

//長押しされた場合
void AFishingRodActor::AdjustPlayerGauge(float DeltaTime)
{
    if (!bIsFishBattle) return;

    PlayerGauge += PlayerGaugeIncreasePerClick * DeltaTime; // 秒単位で増加
    FishGauge += FishGaugeIncreasePerClick * DeltaTime;

    // プレイヤーゲージが魚ゲージを超えないようにクランプ
    PlayerGauge = FMath::Clamp(PlayerGauge, 0.f, FishGauge);
    FishGauge = FMath::Clamp(FishGauge, 1.f, GaugeMax);

    CheckFishBattleState();
}

void AFishingRodActor::OnFishHitEvent()
{
    UE_LOG(LogTemp, Warning, TEXT("Rod: HIT 受信"));
    // --- バトル開始 ---
    bIsFishBattle = true;
    bIsReeling = false;
    bIsCasting = false;

    // 初期ゲージは任意（ここでは半分から開始）
    PlayerGauge = GaugeMax * 0.4f;
    FishGauge = GaugeMax * 0.6f;

    // UIやBPへ通知
    OnStartFishBattle.Broadcast();

    // ルアーはバトル状態へ（移動停止など）
    if (CurrentLure)
    {
        FVector RodTip = RodMesh->GetSocketLocation(TEXT("RodTip"));
        CurrentLure->SetBeingReeled(false, RodTip);
    }
}

void AFishingRodActor::OnReelClick()
{
    if (!bIsFishBattle) return;

    // クリックでプレイヤーゲージが上がる（リスク：上げすぎると負ける）
    PlayerGauge += PlayerGaugeIncreasePerClick;
    PlayerGauge = FMath::Clamp(PlayerGauge, 0.f, GaugeMax);

    // クリックで魚ゲージも少し上がる（プレイヤーの操作は魚の抵抗も増す想定）
    FishGauge += FishGaugeIncreasePerClick;
    FishGauge = FMath::Clamp(FishGauge, 0.f, GaugeMax);

    CheckFishBattleState();
}

void AFishingRodActor::CheckFishBattleState()
{
    if (!bIsFishBattle) return;

    // 失敗条件：プレイヤーのゲージが最大に到達（オーバープレッシャーで切れる）
    if (PlayerGauge >= GaugeMax)
    {
        UE_LOG(LogTemp, Warning, TEXT("FishBattle: Fail - Player gauge max"));
        EndFishBattle(false);
        return;
    }

    // 失敗条件：魚ゲージが 0 になった（魚が疲れ果てて逃げる/ライン切れ）
    if (FishGauge <= 0.f)
    {
        UE_LOG(LogTemp, Warning, TEXT("FishBattle: Fail - Fish gauge 0"));
        EndFishBattle(false);
        return;
    }

    // 失敗条件：プレイヤーゲージが魚ゲージを上回る（プレッシャーでライン切れ）
    if (PlayerGauge > FishGauge)
    {
        UE_LOG(LogTemp, Warning, TEXT("FishBattle: Fail - Player > Fish"));
        EndFishBattle(false);
        return;
    }

    // 成功条件：魚ゲージが満タン（プレイヤーが魚のゲージを最大にして勝つ）
    if (FishGauge >= GaugeMax)
    {
        UE_LOG(LogTemp, Warning, TEXT("FishBattle: Success - Fish gauge max"));
        EndFishBattle(true);
        return;
    }
}

void AFishingRodActor::EndFishBattle(bool bSuccess)
{
    bIsFishBattle = false;

    // UI通知
    OnEndFishBattle.Broadcast(bSuccess);

    if (bSuccess)
    {
        // 魚をスポーン or 既に捕獲済みなら処理
        SpawnCaughtFish();
    }
    else
    {
        // 失敗ならルアーをリセット（魚は逃げる）
        ResetLure();
    }

    // ゲージリセット（任意）
    PlayerGauge = 0.f;
    FishGauge = 0.f;
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