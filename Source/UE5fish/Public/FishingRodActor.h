#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CableComponent.h"
#include "FishingRodActor.generated.h"

class ALureActor;
class AFishActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFishBattleEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFishBattleEndEvent, bool, bSuccess);

UCLASS()
class UE5FISH_API AFishingRodActor : public AActor
{
    GENERATED_BODY()

public:
    AFishingRodActor();
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;
    void ResetRodState();//釣り竿の状態リセット

    /** 竿メッシュ */
    UPROPERTY(VisibleAnywhere)
    USkeletalMeshComponent* RodMesh;

    /** ケーブルで糸を可視化 */
    UPROPERTY(VisibleAnywhere)
    UCableComponent* LineCable;

    /** 現在のルアー */
    UPROPERTY()
    ALureActor* CurrentLure;

    /** 現在釣れた魚 */
    UPROPERTY()
    AFishActor* CaughtFish;

    /** ルアーのクラス */
    UPROPERTY(EditAnywhere)
    TSubclassOf<ALureActor> LureClass;

    /** 魚クラス */
    UPROPERTY(EditAnywhere)
    TSubclassOf<AFishActor> FishClass;

    UFUNCTION()
    void OnFishHitEvent();

    /** ルアーをキャスト */
    void CastToLocation(const FVector& TargetLocation);

    /** 巻き取り開始・停止 */
    UFUNCTION()
    void StartReel();
    UFUNCTION()
    void StopReel();

    /** ヒット・キャッチ処理 */
    void SpawnCaughtFish();
    void ResetLure();

    void AdjustPlayerGauge(float DeltaTime);

    // --- 新：2ゲージ バトル用 API ---
    /** プレイヤーが左クリック（リールクリック）した時に呼ぶ */
    UFUNCTION()
    void OnReelClick();

    /** 外部（UIなど）から現在のゲージを参照できるようにする */
    UPROPERTY(BlueprintReadOnly)
    float PlayerGauge = 0.f;

    UPROPERTY(BlueprintReadOnly)
    float FishGauge = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
    float GaugeMax = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
    float PlayerGaugeIncreasePerClick = 12.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
    float FishGaugeIncreasePerClick = 9.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
    float PlayerGaugeDecayRate = 8.f; // /s

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
    float FishGaugeDecayRate = 6.f; // /s

    /** バトルフラグ */
    UPROPERTY(BlueprintReadOnly)
    bool bIsFishBattle = false;

    /** イベント：バトル開始 / 終了 */
    UPROPERTY(BlueprintAssignable)
    FFishBattleEvent OnStartFishBattle;

    UPROPERTY(BlueprintAssignable)
    FFishBattleEndEvent OnEndFishBattle;
protected:
    /** 状態フラグ */
    bool bIsCasting = false;
    bool bIsReeling = false;
    bool bIsFishBiting = false;
    bool bFishCaught = false;
    bool bIsCharging = false;
    bool bEquipped = false;
    bool Lure = false;


    /** リール進行度 */
    float FishReelProgress = 0.f;

    /** キャストスピード */
    UPROPERTY(EditAnywhere)
    float CastSpeed = 1500.f;
    float CastCharge = 0.f;                 // 現在のチャージ量（0..MaxCastCharge）
    UPROPERTY(EditAnywhere, Category = "Casting")
    float MaxCastCharge = 3.0f;            // 最大チャージ時間（秒など）
    UPROPERTY(EditAnywhere, Category = "Casting")
    float CastChargeRate = 1.0f;           // チャージ速度（増加率）

private:
    FTimerHandle FishBiteTimerHandle;

    void OnFishCaught();        // 巻き上げ完了時
    /** バトル内で勝敗判定 */
    void CheckFishBattleState();

    /** バトル終了処理 */
    void EndFishBattle(bool bSuccess);
};