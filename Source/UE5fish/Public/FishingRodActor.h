#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "CableComponent.h"
#include "FishingRodActor.generated.h"

class ALureActor;
class AFishActor;

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FFishBattleEvent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFishBattleEndEvent, bool, bSuccess);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnFishCaughtUI, FText, FishName, float, Size, UTexture2D*, FishImage, int32, Rarity);

UCLASS()
class UE5FISH_API AFishingRodActor : public AActor
{
    GENERATED_BODY()

public:
    AFishingRodActor();
    virtual void Tick(float DeltaTime) override;
    virtual void BeginPlay() override;
    void ResetRodState();//釣り竿の状態リセット
    void InstantCast();

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

    /** 魚表示ウィジェット */
    UPROPERTY(BlueprintAssignable, Category = "Fishing")
    FOnFishCaughtUI OnFishCaughtUI;

    UPROPERTY(BlueprintAssignable)
    FFishBattleEvent OnStartFishBattle;

    UPROPERTY(BlueprintAssignable)
    FFishBattleEndEvent OnEndFishBattle;

    // データテーブルアセットへの参照
    UPROPERTY(EditDefaultsOnly, Category = "Fishing Data")
    class UDataTable* FishDataTable;

    UFUNCTION()
    void OnFishHitEvent();

    /** ルアーをキャスト */
    UFUNCTION(BlueprintCallable)
    void CastToLocation(const FVector& TargetLocation);

    /** 巻き取り開始・停止 */
    UFUNCTION()
    void StartReel();
    UFUNCTION()
    void StopReel();

    UPROPERTY(EditAnywhere, BlueprintReadWrite) // BPから設定したいなら
    bool bEquipped = false;

    /** ヒット・キャッチ処理 */
    UFUNCTION()
    void SpawnCaughtFish();
    UFUNCTION()
    void SpawnLure();
    UFUNCTION()
    void ResetLure();
    UFUNCTION()
    void SpawnFish();

    UFUNCTION()
    void ReelProgress(float DeltaTime);

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

    // バトル中かどうか
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing|Battle")
    bool bIsFishBattle = false;

    // プレイヤーのリール力 (毎秒のゲージ増加ポテンシャル)
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing|Battle")
    float PlayerReelPower = 15.0f;

    // プレイヤーの現在のゲージ値
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing|Battle")
    float CurrentPlayerGauge = 0.0f;
    //魚ゲージの基本巻き取り速度
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
    float BaseReelSpeed = 3.0f;
    //中央合わせボーナスの強さ
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
    float CenterBoostMultiplier = 10.0f;
    //進行度ボーナスの最大値
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Battle")
    float MaxProgressiveBoost = 8.0f;

    // ゲージの最大値
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Fishing|Battle")
    float MaxGaugeValue = 100.0f;

    void SetFishData(
        const FString& Name,
        float Size,
        int32 Rarity,
        float PlayerGaugeDecayContribution,
        float BaseResistance,
        float MaxResistanceMultiplier
    );

protected:
    /** 状態フラグ */
    bool bIsCasting = false;
    bool bIsReeling = false;
    bool bIsFishBiting = false;
    bool bFishCaught = false;
    bool bIsCharging = false;
    bool bIsPlayerReeling = false;
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

    UPROPERTY(EditAnywhere, Category = "Animations")
    UAnimMontage* RodHitMontage;

private:
    FTimerHandle FishBiteTimerHandle;

    void OnFishCaught();        // 巻き上げ完了時
    /** バトル内で勝敗判定 */
    void CheckFishBattleState();

    /** バトル終了処理 */
    void EndFishBattle(bool bSuccess);

    float CalculateFishResistance();

    /** UIを更新する関数（ゲージの値をBP/ウィジェットに送る） */
    void UpdateBattleGaugeUI(float CurrentValue, float MaxValue);
};