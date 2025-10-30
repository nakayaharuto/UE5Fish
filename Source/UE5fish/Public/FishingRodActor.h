#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishingRodActor.generated.h"

UENUM(BlueprintType)
enum class EFishingState : uint8
{
    Idle,       // 待機中
    Casting,    // キャスト中
    Waiting,    // 魚待ち
    Hooked,     // 魚ヒット
    Reeling,    // 巻き取り中
    Success,    // 成功
    Fail         // 失敗
};

UCLASS()
class UE5FISH_API AFishingRodActor : public AActor
{
    GENERATED_BODY()

public:
    AFishingRodActor();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    /** メッシュ */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
    USkeletalMeshComponent* RodMesh;

    /** 現在の状態 */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fishing")
    EFishingState CurrentState = EFishingState::Idle;

    float CastPower = 0.f;//キャストの力
    float LineTension = 0.f;//糸の張力
    bool bFishOn = false;//魚がヒットしたか判定
    float FishForce = 0.f;
    float ReelSpeed = 0.f;

    // --- 外部操作用 ---
    void StartCasting();    //ボタン押下で溜め開始
    void ReleaseCasting();  //ボタン離しで投げる
    void StartReel();       //リール開始
    void StopReel();        //リール停止

private:
    /** 魚がかかるまでの遅延タイマー */
    FTimerHandle BiteTimerHandle;

    void FishBite();      // 魚ヒット
    void ResetFishing();  // 状態リセット
    void UpdateReeling(float DeltaTime);
};
