#include "LureActor.h"
#include "FishActor.h"
#include "MyCharacter/MyCharacter.h"
#include "FishingRodActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/CableComponent.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"
#include "Engine/World.h"

ALureActor::ALureActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->BodyInstance.bUseCCD = true;

    Mesh->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
    Mesh->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Block);
    Mesh->SetCollisionObjectType(ECC_PhysicsBody);

    Mesh->SetSimulatePhysics(true);
    Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    Mesh->SetMassOverrideInKg(NAME_None, 0.2f);
    Mesh->SetLinearDamping(0.05f);
    Mesh->SetAngularDamping(0.05f);

    // Cable を内部にも持たせておく（視認用、ただし竿の LineCable を使うのが最優先）
    Cable = CreateDefaultSubobject<UCableComponent>(TEXT("Cable"));
    Cable->SetupAttachment(Mesh);
    Cable->bAttachEnd = true;
    Cable->CableLength = 200.f;
    Cable->NumSegments = 12;
    Cable->SetVisibility(false); // 初期は竿側の Cable を使う
}

void ALureActor::BeginPlay()
{
    Super::BeginPlay();
}

void ALureActor::LaunchLure(const FVector& InTarget, float InSpeed)
{
    // 方向をまっすぐに
    StartLocation = GetActorLocation();
    LaunchDirection = (InTarget - StartLocation).GetSafeNormal();
    LaunchSpeed = InSpeed;

    // 少し控えめに投げる（見た目調整）
    Mesh->SetSimulatePhysics(true);
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
    GetWorld()->GetTimerManager().SetTimer(HitTimerHandle, this, &ALureActor::SpawnHitFish, HitDelay, false);

    // ルアーを生成した FishingRod 側があれば、竿の LineCable をこのルアーに繋ぐ処理は FishingRod 側で呼ぶこと。
    // （AFishingRodActor::CastToLocation で LineCable->SetAttachEndTo(CurrentLure, NAME_None) を追加推奨）
}

void ALureActor::EnableAirResistance()
{
    bAirResistanceActive = true;
}

void ALureActor::EndCast()
{
    // 投げ終了（物理OFF、速度を適度に落とす）
    bIsLaunched = false;
    if (Mesh)
    {
        FVector Vel = Mesh->GetPhysicsLinearVelocity();
        Vel *= 0.4f; // 減速
        Mesh->SetPhysicsLinearVelocity(Vel);
        Mesh->SetSimulatePhysics(false);
    }

    // Broadcast 水面ヒットの代替イベント（必要なら）
    OnHitWater.Broadcast();
}

void ALureActor::SpawnHitFish()
{
    if (HitFish) return; // 既に魚がいるならスキップ

    // ルアーの近くに魚をスポーンして見せる
    FVector SpawnLoc = GetActorLocation() + FVector(FMath::FRandRange(60.f, 140.f), 0.f, -20.f);
    FActorSpawnParameters Params;
    Params.Owner = GetOwner(); // オーナーは竿のはず
    UWorld* W = GetWorld();
    if (!W) return;

    HitFish = W->SpawnActor<AFishActor>(AFishActor::StaticClass(), SpawnLoc, FRotator::ZeroRotator, Params);
    if (HitFish)
    {
        HitFish->ShowFish(); // ShowFish() は既存実装を利用
    }

    // 外部で検知したい場合向けにデリゲート発火
    OnFishHit.Broadcast();
}

void ALureActor::SetBeingReeled(bool bReeling, const FVector& ReelTargetIn)
{
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

void ALureActor::ResetLure()
{
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
        HitFish->Destroy();
        HitFish = nullptr;
    }

    // タイマークリア
    GetWorld()->GetTimerManager().ClearTimer(HitTimerHandle);
    GetWorld()->GetTimerManager().ClearTimer(AirResistTimer);

    // 所有者の竿があれば初期化をお願いする（Owner は AFishingRodActor）
    if (AFishingRodActor* Rod = Cast<AFishingRodActor>(GetOwner()))
    {
        Rod->ResetRodState();
    }
}

void ALureActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    // もし魚アクターとぶつかったら（既存の挙動を踏襲）
    AFishActor* Fish = Cast<AFishActor>(OtherActor);
    if (Fish)
    {
        // 外部に発火（例えば竿側も Bind しているなら反応する）
        OnFishHit.Broadcast();

        // 表示は FishingRod / SpawnHitFish を優先してるのでここは軽めに扱う
        // 例えば魚にアタックさせる等の拡張はここに追加
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
            HitFish->Destroy();
            HitFish = nullptr;
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

void ALureActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    if (!Mesh) return;

    LaunchTime += DeltaTime;

    // 飛行中の処理（空気抵抗・距離/時間で投げ終了）
    if (bIsLaunched && !bIsBeingReeled)
    {
        if (bAirResistanceActive)
        {
            FVector Vel = Mesh->GetPhysicsLinearVelocity();
            float DynamicResistance = FMath::Clamp(AirResistance + (LaunchTime - 0.25f) * 0.002f, 0.f, 0.05f);
            FVector DampedVel = Vel * (1.f - DynamicResistance);
            Mesh->SetPhysicsLinearVelocity(DampedVel);
        }

        // 距離チェックまたは時間チェックで EndCast
        float DistFromStart = FVector::Distance(StartLocation, GetActorLocation());
        if (DistFromStart >= MaxCastDistance || LaunchTime >= MaxCastTime)
        {
            EndCast();
        }
    }

    // リール処理（一定速度で移動）
    if (bIsBeingReeled)
    {
        ReelStep(DeltaTime);
    }

    // 糸の見た目更新（Owner が竿なら竿先の位置を渡す）
    if (AActor* OwnerActor = GetOwner())
    {
        if (AFishingRodActor* Rod = Cast<AFishingRodActor>(OwnerActor))
        {
            FVector RodTip = Rod->RodMesh->GetSocketLocation(TEXT("RodTip"));
            UpdateFishingLine(RodTip, GetActorLocation());
        }
    }
}

void ALureActor::UpdateFishingLine(const FVector& Start, const FVector& End)
{
    // 現在は内部 Cable は使っていない（竿の LineCable を使う前提）。
    // 必要ならここで Cable->SetWorldLocation、Cable->CableLength の更新を入れる。
}
