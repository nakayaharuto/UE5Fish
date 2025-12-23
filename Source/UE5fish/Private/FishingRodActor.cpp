#include "FishingRodActor.h"
#include "LureActor.h"
#include "MyCharacter/MyCharacter.h"
#include "FishActor.h"
#include "FishData.h"
#include "Engine/EngineTypes.h"
#include "Engine/DataTable.h"
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
    bHasCalledEndBattle = false;
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

    if (CaughtFish)
    {
        CaughtFish->Destroy();
        CaughtFish = nullptr;
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
    UE_LOG(LogTemp, Warning, TEXT("いり"));
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
    if (!bIsFishBattle) return;

   
    // 魚の抵抗力を動的に計算 (PlayerGaugeの計算にのみ使用)
    float DynamicResistance = CalculateFishResistance();

    // 1. 中央合わせボーナス計算 (PlayerGaugeの位置による効率ボーナス)
    float CenterDeviation = FMath::Abs(PlayerGauge - (GaugeMax / 2.0f));
    float MaxDeviation = GaugeMax / 2.0f;
    float CenterBonus = 1.0f - (CenterDeviation / MaxDeviation); // 中央で最大1.0、端で最小0.0

    PlayerGauge += PlayerReelPower * DeltaTime;
    PlayerGauge = FMath::Clamp(PlayerGauge, 0.0f, GaugeMax);

    // 🐠 魚ゲージの調整 (純粋な増加ロジック)

    float ProgressiveBoost = (FishGauge / GaugeMax) * MaxProgressiveBoost;

    float ReelIncrease = BaseReelSpeed + (CenterBonus * CenterBoostMultiplier) + ProgressiveBoost;

    float FinalFishGaugeChange = ReelIncrease * DeltaTime;

    // FishGaugeの更新
    FishGauge += FinalFishGaugeChange;
    FishGauge = FMath::Clamp(FishGauge, 0.0f, GaugeMax);

    // UIの更新 (実装が必要な関数)
    UpdateBattleGaugeUI(PlayerGauge, GaugeMax);

    //終了判定
    CheckFishBattleState();
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
          

            if (bIsFishBiting && IsValid(CaughtFish))
            {
                OnFishCaught(); // 🎣 魚を釣り上げる処理
            }
           
            if (IsValid(CurrentLure))
            {
                CurrentLure->SetBeingReeled(false, RodTip);
            }

            //リールフラグをオフ
            bIsReeling = false;
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
        // 1. プレイヤーゲージの減衰 (変更なし)
        if (!bIsPlayerReeling)
        {
            PlayerGauge -= PlayerGaugeDecayRate * DeltaTime;
        }

        // 2. 魚ゲージの純粋な減少ロジック
        if (!bIsPlayerReeling) // bIsPlayerReeling が false の場合のみ減衰
        {
            float CurrentDynamicResistance = CalculateFishResistance();
            // 魚ゲージの減少は、魚の抵抗力に比例して強くする
            float FishDecayRate = 2.0f; // 基本減衰速度
            float DecayMultiplier = 0.5f; // 抵抗力の影響を調整する係数

            // 魚の抵抗力を利用して減衰力を決定
            float FinalDecayRate = FishDecayRate + (CurrentDynamicResistance * DecayMultiplier);

            FishGauge -= FinalDecayRate * DeltaTime;
        }

        // 3. 全ゲージのクランプ
        PlayerGauge = FMath::Clamp(PlayerGauge, 0.f, GaugeMax);
        FishGauge = FMath::Clamp(FishGauge, 0.f, GaugeMax);

        //終了判定
        CheckFishBattleState();
    }
}

// 魚の抵抗力を計算
float AFishingRodActor::CalculateFishResistance()
{
    // 釣れた魚の参照が存在することを確認 (CaughtFishをAMyFishActor型で保持していると仮定)
    if (CaughtFish)
    {
        // 魚オブジェクトに計算を委譲
        return CaughtFish->GetCurrentDynamicResistance(FishGauge, GaugeMax);
    }
    // 魚がいない場合のデフォルト値
    return 1.0f;
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

    //プレイヤーのアニメーション
    if (AMyCharacter* MyChar = Cast<AMyCharacter>(GetOwner()))
    {
        MyChar->PlayFishHitAnimation();
    }

    if (RodHitMontage && RodMesh->GetAnimInstance())
    {
        RodMesh->GetAnimInstance()->Montage_Play(RodHitMontage);
    }

    if (!IsValid(CaughtFish))
    {
        // 魚データテーブルを参照し、ランダムな魚をスポーンさせる
        // FishActorのスポーンと、そのパラメータ（サイズ、レア度など）の設定を行います。
        SpawnFish();
    }

    // --- バトル開始 ---
    bIsFishBattle = true;
    bIsReeling = false;
    bIsCasting = false;


    float InitialGaugePercentage = 0.15f;
    // 初期ゲージは任意（ここでは半分から開始）
    PlayerGauge = GaugeMax * InitialGaugePercentage;
    FishGauge = GaugeMax * InitialGaugePercentage;

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

    // 敗北判定 2: プレイヤーゲージが MAX になった（ラインブレイク）
    if (PlayerGauge >= GaugeMax)
    {
        UE_LOG(LogTemp, Error, TEXT("LINE BREAK! EndBattle(false) called."));
        EndFishBattle(false);
        return;
    }

    // 勝利判定: 魚ゲージが MAX になった（リールを巻ききった！）
    if (FishGauge >= GaugeMax)
    {
        EndFishBattle(true);
        return;
    }

    // 敗北判定 1: プレイヤーゲージが 0 になった（魚に逃げられた）
    if (PlayerGauge <= 0.0f)
    {
        EndFishBattle(false);
        return;
    }

    // 敗北判定 3:魚ゲージが 0 になった（魚に逃げられた）
    if (FishGauge <= 0.0f)
    {
        UE_LOG(LogTemp, Warning, TEXT("FISH GAUGE ZERO! EndBattle(false) called (Fish escaped)."));
        EndFishBattle(false);
        return;
    }
}

void AFishingRodActor::UpdateBattleGaugeUI(float CurrentValue, float MaxValue)
{
    // デバッグログで値を確認する (一時的な処理)
    //UE_LOG(LogTemp, Log, TEXT("UI Update: Gauge = %.2f / %.2f"), CurrentValue, MaxValue);
}

void AFishingRodActor::EndFishBattle(bool bSuccess)
{
    if (bHasCalledEndBattle) return;
    bHasCalledEndBattle = true;

    bIsPlayerReeling = false;

    if (bSuccess)
    {
        // 成功の場合、リールアップを開始し、ルアーを竿先まで引き戻す
        if (CurrentLure)
        {
            // 魚は OnFishCaught で処理されるため、ルアーだけリールアップ
            bIsReeling = true;
            StartReel();
           
        }
    }
    else
    {
        // 失敗の場合もリールアップ処理を行う
        if (CurrentLure)
        {
            bIsReeling = true;
            //UE_LOG(LogTemp, Warning, TEXT("来てます！来てますよぉ！"));
            StartReel();
        }
        else
        {
            // CurrentLure が NULL の場合は即時リセット
            ResetLure();
        }
    }
    bIsFishBattle = false;

    // ゲージリセット（任意）
    PlayerGauge = 0.f;
    FishGauge = 0.f;

    // UI通知
    OnEndFishBattle.Broadcast(bSuccess);
}

void AFishingRodActor::OnFishCaught()
{
    if (!IsValid(CaughtFish) || !bIsFishBattle)return;

    bFishCaught = true;     //成功フラグ
    bIsFishBattle = false; // 釣りバトル終了
    bIsPlayerReeling = false;
    // 自動リールアップを開始するフラグを立てる
    bIsReeling = true;
    bIsFishBiting = false;  //バトルフラグをリセット
}

void AFishingRodActor::SpawnFish()
{
    // データテーブルとスポーン対象クラスの基本的なチェック
    if (!FishDataTable || !FishClass) return;

    TArray<FFishingFishData*> AllRows;
    FishDataTable->GetAllRows<FFishingFishData>(TEXT("SpawnLogic"), AllRows);

    if (AllRows.Num() == 0) return;

    // --- 確率の「重み」を計算 ---
    float TotalWeight = 0.0f;
    TArray<float> RowWeights;

    for (FFishingFishData* Row : AllRows)
    {
        // レア度が高いほど重みを小さくする計算式
        // 例: レア1=100.0, レア2=20.0, レア3=4.0 ... 
        float Weight = 100.0f / FMath::Max(1.0f, FMath::Pow(5.0f, (float)(Row->Rarity - 1)));
        RowWeights.Add(Weight);
        TotalWeight += Weight;
    }

    // --- 抽選実行 ---
    float RandomPoint = FMath::FRandRange(0.0f, TotalWeight);
    float CurrentWeightSum = 0.0f;
    FFishingFishData* SelectedFishData = nullptr;

    for (int32 i = 0; i < AllRows.Num(); i++)
    {
        CurrentWeightSum += RowWeights[i];
        if (RandomPoint <= CurrentWeightSum)
        {
            SelectedFishData = AllRows[i];
            break;
        }
    }

    if (SelectedFishData)
    {
        // 3. サイズの決定 (MinSizeとMaxSizeの間でランダム)
        float ActualSize = FMath::FRandRange(SelectedFishData->MinSize, SelectedFishData->MaxSize);

        // 4. スポーン位置と回転の決定 (ルアーが存在すればルアーの位置)
        FVector SpawnLocation = CurrentLure ? CurrentLure->GetActorLocation() : GetActorLocation();
        FRotator SpawnRotation = CurrentLure ? CurrentLure->GetActorRotation() : GetActorRotation();

        FActorSpawnParameters Params;
        Params.Owner = this;
        Params.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

        // AFishActor をスポーンし、CaughtFish に代入
        CaughtFish = GetWorld()->SpawnActor<AFishActor>(FishClass, SpawnLocation, SpawnRotation, Params);

        if (CaughtFish)
        {
            // メッシュアセットの同期ロード
            //class USkeletalMesh* FishMesh = SelectedFishData->FishMeshAsset.LoadSynchronous();

            // A. SetFishData を呼び出し、魚の基本情報を設定
            // SetFishData の引数定義は、この5つに合うように AFishActor.h で宣言が必要です。
            CaughtFish->SetFishData(
                SelectedFishData->FishName,
                ActualSize,
                SelectedFishData->Rarity,
                SelectedFishData->PlayerGaugeDecayContribution,
                SelectedFishData->BaseResistance,
                SelectedFishData->MaxResistanceMultiplier,
                SelectedFishData->UITexture
            );

            // 例外: FFishData にある Decay 係数を、AFishActor の UPROPERTY に直接設定
            // AFishActor::PlayerGaugeDecayContribution が UPROPERTY であることが前提
            CaughtFish->PlayerGaugeDecayContribution = SelectedFishData->PlayerGaugeDecayContribution;

            UE_LOG(LogTemp, Warning, TEXT("Fish Spawned: %s (Size: %.1f cm)"), *SelectedFishData->FishName, ActualSize);
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Failed to spawn AFishActor."));
        }
    }
}