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
}

void AFishingRodActor::SpawnCaughtFish()
{
    UE_LOG(LogTemp, Warning, TEXT("SpawnCaughtFish called"));
    if (CurrentLure)
    {
        UE_LOG(LogTemp, Warning, TEXT("CurrentLure exists so skipping"));
        return;
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

    // ルアーを安全にリセット（Destroyする前にケーブルの参照を解除）
    ResetLure();

    // 竿を表示状態に（必要であれば）
    SetActorHiddenInGame(false);
}

void AFishingRodActor::SpawnLure()
{
    if (CurrentLure) return;

    FVector RodTip = RodMesh->GetSocketLocation(TEXT("RodTip"));
    FRotator Rot = RodMesh->GetSocketRotation(TEXT("RodTip"));

    FActorSpawnParameters Params;
    Params.Owner = this;
    Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    CurrentLure = GetWorld()->SpawnActor<ALureActor>(LureClass, RodTip, Rot, Params);

    if (CurrentLure)
    {
        CurrentLure->OnFishHit.AddDynamic(this, &AFishingRodActor::OnFishHitEvent);

        if (LineCable)
        {
            LineCable->SetAttachEndToComponent(CurrentLure->Mesh);
            LineCable->SetVisibility(true);
            LineCable->CableLength = 50.f;
        }
    }
}

void AFishingRodActor::CastToLocation(const FVector& InTargetLocation)
{
    UE_LOG(LogTemp, Warning, TEXT("Rod: CastToLocation ENTRY POINT. bEquipped: %d, bIsCasting: %d"), bEquipped, bIsCasting);
    if (!bEquipped || bIsCasting) return;
    bIsCasting = true;

    FVector RodTipLocation = RodMesh->GetSocketLocation(FName("RodTip"));
    FRotator CastRotation = (InTargetLocation - RodTipLocation).Rotation();

    FActorSpawnParameters SpawnParams;
    SpawnParams.Owner = this;
    SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

    // すでにあるルアーは Reuse する or 消してもOK
    if (!CurrentLure)
    {
        CurrentLure = GetWorld()->SpawnActor<ALureActor>(
            LureClass,
            RodTipLocation,
            CastRotation,
            SpawnParams
        );

        if (CurrentLure)
        {
            CurrentLure->OnFishHit.AddDynamic(this, &AFishingRodActor::OnFishHitEvent);
        }
    }

    if (!CurrentLure) return;

    // ケーブル接続をMeshに
    if (LineCable)
    {
        LineCable->SetAttachEndToComponent(CurrentLure->Mesh);
        LineCable->SetVisibility(true);
        LineCable->CableLength = FVector::Distance(RodTipLocation, InTargetLocation);
    }

    // 竿先にリセット
    CurrentLure->SetActorLocation(RodTipLocation);
    CurrentLure->Mesh->SetSimulatePhysics(true);

    // UIゲージを使わない場合、CastSpeedを固定値にして飛ばす
    float FinalCastSpeed = CastSpeed; // 例：1500.f

    UE_LOG(LogTemp, Warning, TEXT("Rod: CastToLocation called. FinalCastSpeed is: %f"), FinalCastSpeed);

    if (!CurrentLure) return; // CurrentLure が NULL でないか再確認の目印にもなる

    CurrentLure->LaunchLure(InTargetLocation, FinalCastSpeed);

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

void AFishingRodActor::ReelProgress(float DeltaTime)
{
    if (!bIsFishBattle)
    {
        return;
    }

    // 魚の抵抗力を動的に計算 (実装が必要な関数)
    float DynamicResistance = CalculateFishResistance();

    // プレイヤーの力 (PlayerReelPower) と 魚の抵抗 (DynamicResistance) の差分でゲージを増減させる
    // プレイヤーの力は PlayerReelPower を使う想定
    float GaugeChange = (PlayerReelPower - DynamicResistance) * DeltaTime;

    // ゲージ増減の適用
    PlayerGauge += GaugeChange;

    // ゲージ値をクランプ
    PlayerGauge = FMath::Clamp(PlayerGauge, 0.0f, GaugeMax);

    // UIの更新 (実装が必要な関数)
    UpdateBattleGaugeUI(PlayerGauge, GaugeMax);

    // 終了判定
    if (PlayerGauge >= GaugeMax)
    {
        // 釣り上げ成功！
        EndFishBattle(true);
    }
    // ここでは PlayerGauge が 0 になると敗北とする (ラインブレイク/魚逃走)
    else if (PlayerGauge <= 0.0f)
    {
        // 逃げられた/ラインブレイク！
        EndFishBattle(false);
    }
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

// 魚の抵抗力を計算
float AFishingRodActor::CalculateFishResistance()
{
    float BaseResistance = 5.0f; // 基本抵抗力
    float TensionMultiplier = 15.0f; // 最大時の抵抗係数

    // 魚ゲージが高いほど抵抗力が高くなるロジック (例)
    float DynamicResistance = BaseResistance + (FishGauge / GaugeMax) * TensionMultiplier;

    return DynamicResistance;
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

void AFishingRodActor::InstantCast()
{
    if (!bEquipped || bIsCasting) return;

    FVector RodTip = RodMesh->GetSocketLocation(TEXT("RodTip"));
    FVector Target = RodTip + (RodMesh->GetForwardVector() * 2000.f);

    CastToLocation(Target); // ← 既存の関数を使う
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

    // ① 魚ゲージが 0 → 敗北
    if (FishGauge <= 0)
    {
        UE_LOG(LogTemp, Warning, TEXT("FAIL: FishGauge reached 0"));
        EndFishBattle(false);  // false = fail
        return;
    }

    // ② 魚ゲージ MAX → 勝利
    if (FishGauge >= GaugeMax)
    {
        UE_LOG(LogTemp, Warning, TEXT("SUCCESS: FishGauge reached MAX"));
        EndFishBattle(true);   // true = success
        return;
    }

    // ③ プレイヤーゲージ MAX → 敗北
    if (PlayerGauge >= GaugeMax)
    {
        UE_LOG(LogTemp, Warning, TEXT("FAIL: PlayerGauge reached MAX"));
        EndFishBattle(false);  // false = fail
        return;
    }
}

void AFishingRodActor::UpdateBattleGaugeUI(float CurrentValue, float MaxValue)
{
    // デバッグログで値を確認する (一時的な処理)
    UE_LOG(LogTemp, Log, TEXT("UI Update: Gauge = %.2f / %.2f"), CurrentValue, MaxValue);
}

void AFishingRodActor::EndFishBattle(bool bSuccess)
{
    bIsFishBattle = false;

    // UI通知
    OnEndFishBattle.Broadcast(bSuccess);

    if (bSuccess)
    {
        // 魚をスポーン or 既に捕獲済みなら処理
        //SpawnCaughtFish();
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