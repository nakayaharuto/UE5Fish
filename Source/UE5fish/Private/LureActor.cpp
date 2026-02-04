#include "LureActor.h"
#include "FishActor.h"
#include "MyCharacter/MyCharacter.h"
#include "FishingRodActor.h"
#include "Components/StaticMeshComponent.h"
#include "CableComponent.h"
#include "FishingRodActor.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

ALureActor::ALureActor()
{
    PrimaryActorTick.bCanEverTick = true;
    bIsFishHit = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

    Cable = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
    Cable->SetupAttachment(Mesh);
    
    if (!HasAnyFlags(RF_ClassDefaultObject))
    {
        // メッシュの物理設定
        Mesh->BodyInstance.bUseCCD = true;
        Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
        Mesh->SetMassOverrideInKg(NAME_None, 0.2f);
        Mesh->SetLinearDamping(0.05f);
        Mesh->SetAngularDamping(0.05f);
        Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
        Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
        Mesh->SetCollisionObjectType(ECC_PhysicsBody);

        // ケーブルの設定
        Cable->bAttachEnd = true;
        Cable->CableLength = 200.f;
        Cable->NumSegments = 12;
    }
    Cable->SetVisibility(false); // 初期は竿側の Cable を使う
}

void ALureActor::BeginPlay()
{
    Super::BeginPlay();

}

void ALureActor::LaunchLure(const FVector& InTarget, float InSpeed)
{
    bIsFishHit = false;
    bIsLaunched = true;
    bIsInWaterVolume = false;
    bCanCheckLanding = false; // ★これが false だと Tick 内の判定が一生動きません
    bIsBeingReeled = false;

    FTimerHandle LandingEnableTimer;
    GetWorld()->GetTimerManager().SetTimer(LandingEnableTimer, [this]() {
        bCanCheckLanding = true;
        }, 0.2f, false);

    // 方向をまっすぐに
    StartLocation = GetActorLocation();
    LaunchDirection = (InTarget - StartLocation).GetSafeNormal();
    LaunchSpeed = InSpeed;
    

    // 少し控えめに投げる（見た目調整）
    Mesh->SetSimulatePhysics(true);
    Mesh->BodyInstance.bNotifyRigidBodyCollision = true;
    Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Mesh->AddImpulse(LaunchDirection * LaunchSpeed, NAME_None, true);

    LaunchTime = 0.f;
    bIsLaunched = true;
    bIsBeingReeled = false;
    bAirResistanceActive = false;

    // 一定時間後に空気抵抗を有効にする（既存の挙動を踏襲）
    GetWorld()->GetTimerManager().SetTimer(AirResistTimer, this, &ALureActor::EnableAirResistance, 0.25f, false);

    // ランダムヒットタイマー（既存スクリプトの意図を残す）
    float HitDelay = FMath::FRandRange(HitDelayRange.X, HitDelayRange.Y);
}

void ALureActor::EnableAirResistance()
{
    bAirResistanceActive = true;
}

void ALureActor::EndCast()
{
    if (!bIsLaunched) return; // 二重実行防止
    bIsLaunched = false;

    TArray<AActor*> OverlappingActors;
    GetOverlappingActors(OverlappingActors);

    //投げ終了（物理OFF、速度を適度に落とす）
    if (Mesh)
    {
        Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
        Mesh->SetSimulatePhysics(false);
    }

    bool bFinalWaterCheck = false;
    for (AActor* Actor : OverlappingActors)
    {
        if (Actor && Actor->ActorHasTag(FName("WaterArea")))
        {
            bFinalWaterCheck = true;
            break;
        }
    }

    if (bFinalWaterCheck) // 飛んでいる最中のフラグではなく、今現在の重なりで判定
    {
        // --- 【水面：バトル開始】 ---
        float HitDelay = FMath::FRandRange(HitDelayRange.X, HitDelayRange.Y);
        GetWorld()->GetTimerManager().SetTimer(HitTimerHandle, this, &ALureActor::OnFishHitConfirmed, HitDelay, false);

        UE_LOG(LogTemp, Warning, TEXT("Lure landed safely in WATER! Battle timer: %f"), HitDelay);
    }
    else
    {
        // --- 【陸地】 ---
        UE_LOG(LogTemp, Warning, TEXT("Lure on Ground. Auto-reeling back..."));
        if (AFishingRodActor* Rod = Cast<AFishingRodActor>(GetOwner()))
        {
            // 1. 竿側のフラグを「リール中」に書き換える（もしあれば）
            Rod->bIsCasting = false; 

            //ルアーを回収モードにする
            SetBeingReeled(true, Rod->GetActorLocation());

            UE_LOG(LogTemp, Warning, TEXT("Auto-reel started towards: %s"), *Rod->GetActorLocation().ToString());
        }
    }
    // Broadcast 水面ヒットの代替イベント（必要なら）
    OnHitWater.Broadcast();
}

void ALureActor::SpawnHitFish()
{
    OnFishHitConfirmed();
}

void ALureActor::SetBeingReeled(bool bReeling, const FVector& ReelTargetIn)
{
    UE_LOG(LogTemp, Warning, TEXT("Lure::SetBeingReeled called. bReeling: %s"), bReeling ? TEXT("True") : TEXT("False"));
    bIsBeingReeled = bReeling;
    ReelTarget = ReelTargetIn;

    if (bIsBeingReeled)
    {
        // リール開始時は物理OFFして自分で移動させる（一定速度）
        if (Mesh) Mesh->SetSimulatePhysics(false);
        // Cable は竿側で終端がこのアクターに繋がれていれば見た目は保たれる
    }
    else
    {
        // リール停止：スピードを一度リセット
        if (Mesh)
        {
            Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
            Mesh->SetSimulatePhysics(false);
        }
    }
}

void ALureActor::TryFishHit()
{
    UE_LOG(LogTemp, Warning, TEXT("TryFishHit called"));
}

void ALureActor::ResetLure()
{

    bIsFishHit = false;

    // ルアーを竿先に戻し、状態を初期化
    bIsLaunched = false;
    bIsBeingReeled = false;
    LaunchTime = 0.f;

    if (Mesh)
    {
        Mesh->SetSimulatePhysics(false);
        Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
        Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);
        Mesh->SetVisibility(true);
    }

    // 魚がいたら消す（今回の優先は表示→消去）
    if (HitFish)
    {
        //HitFish->Destroy();
        HitFish = nullptr;
    }

    // タイマークリア
    GetWorld()->GetTimerManager().ClearTimer(HitTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(AirResistTimer);

    // 所有者の竿があれば初期化をお願いする（Owner は AFishingRodActor）
    if (AFishingRodActor* Rod = Cast<AFishingRodActor>(GetOwner()))
    {
        //Rod->ResetRodState();
    }

   
}

void ALureActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    if (OtherActor && OtherActor->ActorHasTag(FName("WaterArea")))
    {
        // まだ投げている最中なら、即座にバトル判定へ
        if (bIsLaunched && !bIsFishHit)
        {
            UE_LOG(LogTemp, Warning, TEXT("Water detected! Forcing EndCast."));

            // 物理を止めて位置を固定
            if (Mesh)
            {
                Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
                Mesh->SetSimulatePhysics(false);
            }

            bIsLaunched = false;
            bIsInWaterVolume = true;

            // 抽選開始（即ヒットさせたいなら HitDelay を 0.1f とかに設定）
            float HitDelay = FMath::FRandRange(HitDelayRange.X, HitDelayRange.Y);
            GetWorld()->GetTimerManager().SetTimer(HitTimerHandle, this, &ALureActor::OnFishHitConfirmed, HitDelay, false);
        }
    }
}

void ALureActor::NotifyActorEndOverlap(AActor* OtherActor)
{
    Super::NotifyActorEndOverlap(OtherActor);

    if (!bIsLaunched) return;

    if (OtherActor && OtherActor->ActorHasTag(FName("WaterArea")))
    {
        bIsInWaterVolume = false;
    }
}

void ALureActor::OnFishHitConfirmed()
{
    if (bIsFishHit) return;

    GetWorld()->GetTimerManager().ClearTimer(AutoReelTimerHandle);

    bIsFishHit = true;
    // デバッグ用：画面左上に小さく表示
    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(
            1,                // メッセージID（同じIDを使うと常に同じ位置に表示）
            5.f,              // 表示時間（秒）
            FColor::Yellow,   // 文字色
            TEXT("Hit判定")   // 表示テキスト
        );
    }

    FActorSpawnParameters Params;
    HitFish = GetWorld()->SpawnActor<AFishActor>(
        FishClass,
        GetActorLocation(),
        GetActorRotation(),
        Params
    );

    if (HitFish)
    {
        // 竿に通知
        OnFishHit.Broadcast();
        UE_LOG(LogTemp, Warning, TEXT("Fish HIT CONFIRMED!"));
    }
}

void ALureActor::ReelStep(float DeltaTime)
{
    if (!bIsBeingReeled) return;

    FVector Current = GetActorLocation();
    FVector Dir = (ReelTarget - Current);
    float Dist = Dir.Size();
    if (Dist <= ReelArrivalThreshold)
    {
        // 竿に到着
        SetActorLocation(ReelTarget);
        bIsBeingReeled = false;

        // 魚がついていれば消す（まずは見た目優先）
        if (HitFish)
        {
            
        }

        // 竿の初期化を呼ぶ（所有者が竿のはず）
        if (AFishingRodActor* Rod = Cast<AFishingRodActor>(GetOwner()))
        {
            // 竿内部でケーブルを切り離したり表示を隠したり ResetRodState をやる
            Rod->ResetRodState();
        }

        return;
    }

    Dir /= Dist; // 正規化
    FVector NewPos = Current + Dir * ReelSpeed * DeltaTime;
    SetActorLocation(NewPos);
}

void ALureActor::NotifyHit(UPrimitiveComponent* MyComp, AActor* Other, UPrimitiveComponent* OtherComp, bool bSelfMoved, FVector HitLocation, FVector HitNormal, FVector NormalImpulse, const FHitResult& Hit)
{
    Super::NotifyHit(MyComp, Other, OtherComp, bSelfMoved, HitLocation, HitNormal, NormalImpulse, Hit);

    // 投げている最中に何かに当たったら
    if (bIsLaunched && bCanCheckLanding && !bIsBeingReeled)
    {
        // それが水（Trigger）ではない固形物（地面など）なら
        if (Other && !Other->ActorHasTag(FName("WaterArea")))
        {
            UE_LOG(LogTemp, Warning, TEXT("Lure HIT something solid: %s. Calling EndCast."), *Other->GetName());
            EndCast();
        }
    }
}

void ALureActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // 投げ中は物理処理
    if (bIsLaunched && !bIsBeingReeled)
    {
        if (bAirResistanceActive)
        {
            FVector Vel = Mesh->GetPhysicsLinearVelocity();
            float Damping = FMath::Clamp(AirResistance + (LaunchTime - 0.25f) * 0.002f, 0.f, 0.05f);
            Mesh->SetPhysicsLinearVelocity(Vel * (1.f - Damping));
        }

        if (bIsLaunched && bCanCheckLanding && !bIsBeingReeled)
        {
            float CurrentSpeed = Mesh->GetPhysicsLinearVelocity().Size();

            // 速度が非常に低い（着地した）状態が一定時間続いたら EndCast
            if (CurrentSpeed < 30.0f)
            {
                UE_LOG(LogTemp, Warning, TEXT("Lure: Stopped moving. Calling EndCast."));
                EndCast();
            }
        }
    }

    // リール中は一定速度で巻く
    if (bIsBeingReeled)
        ReelStep(DeltaTime);
}
