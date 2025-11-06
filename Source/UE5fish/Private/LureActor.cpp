#include "LureActor.h"
#include "FishActor.h"
#include "MyCharacter/MyCharacter.h"
#include "FishingRodActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

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
}

void ALureActor::BeginPlay()
{
    Super::BeginPlay();
}

void ALureActor::ResetLure()
{
    bIsLaunched = false;
    bIsBeingReeled = false;
    LaunchTime = 0.f;

    Mesh->SetSimulatePhysics(true);
    Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

    // プレイヤーの竿の先端（初期位置）に戻す
    if (AActor* OwnerActor = GetOwner())
    {
        FVector SocketLoc = OwnerActor->GetActorLocation();
        SetActorLocation(SocketLoc);
    }
}

void ALureActor::LaunchLure(const FVector& InTarget, float InSpeed)
{
    StartLocation = GetActorLocation();
    LaunchDirection = (InTarget - StartLocation).GetSafeNormal();
    LaunchSpeed = InSpeed;

    Mesh->SetSimulatePhysics(true);
    Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
    Mesh->AddImpulse(LaunchDirection * LaunchSpeed);

    LaunchTime = 0.f;
    bIsLaunched = true;
    bIsBeingReeled = false;

    // 空気抵抗は投げてからしばらくOFFにする
    bAirResistanceActive = false;
    GetWorld()->GetTimerManager().SetTimer(AirResistTimer, this, &ALureActor::EnableAirResistance, 0.3f, false);
}

void ALureActor::EnableAirResistance()
{
    bAirResistanceActive = true;
}

void ALureActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (!Mesh) return;

    // オーナーが竿（キャラクター）なら
    if (AActor* OwnerActor = GetOwner())
    {
        FVector RodTipLocation = OwnerActor->GetActorLocation(); // ← 本来は「竿先ソケット」の位置
        float CurrentLineLength = FVector::Distance(RodTipLocation, GetActorLocation());

        // 糸の長さを保存して他で使えるように
        CurrentLineLength = FMath::Clamp(CurrentLineLength, 0.f, MaxDistance);

        // デバッグ表示（長さ確認用）
        GEngine->AddOnScreenDebugMessage(-1, 0.f, FColor::Cyan,
            FString::Printf(TEXT("Line Length: %.2f"), CurrentLineLength));

        // 糸をスプラインやNiagaraに反映したい場合はここで更新
        UpdateFishingLine(RodTipLocation, GetActorLocation());
    }

    LaunchTime += DeltaTime;

    FVector Velocity = Mesh->GetPhysicsLinearVelocity();

    // ====== 飛行中 ======
    if (bIsLaunched && !bIsBeingReeled)
    {
        if (bAirResistanceActive) // ← 0.3秒経過後のみ抵抗を適用
        {
            // 経過時間に応じて空気抵抗を緩やかに増やす
            float DynamicResistance = FMath::Clamp(AirResistance + (LaunchTime - 0.3f) * 0.002f, 0.f, 0.05f);
            FVector DampedVel = Velocity * (1.f - DynamicResistance);
            Mesh->SetPhysicsLinearVelocity(DampedVel);
        }

        // 一定距離でブレーキを少し強める
        float Distance = FVector::Distance(StartLocation, GetActorLocation());
        if (Distance > MaxDistance)
        {
            FVector Reduced = Velocity * 0.95f; // ゆるくブレーキ
            Mesh->SetPhysicsLinearVelocity(Reduced);
            bIsLaunched = false;
        }
    }

    // ====== リール中 ======
    if (bIsBeingReeled)
    {
        FVector DirToRod = (ReelTarget - GetActorLocation());
        float Dist = DirToRod.Length();

        if (Dist < 80.f)
        {
            // 🎣 ルアーが竿先に戻ったとき
            FVector SnapPos = ReelTarget - DirToRod.GetSafeNormal() * 10.f;
            SetActorLocation(SnapPos);
            Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
            Mesh->SetSimulatePhysics(false);
            Mesh->SetVisibility(false);

            // 以後リール処理を止める
            bIsBeingReeled = false;
            bIsLaunched = false;
            
            // 🎣 釣り竿初期化呼び出し
            if (AActor* OwnerActor = GetOwner())
            {
                // プレイヤーキャラクターをキャスト
                if (AMyCharacter* MyChar = Cast<AMyCharacter>(OwnerActor))
                {
                    if (MyChar->FishingRod)
                    {
                        Mesh->SetSimulatePhysics(false);
                        Mesh->SetVisibility(false);
                    }
                }
            }
            return;
        }

        DirToRod.Normalize();
        float Attenuation = FMath::Clamp(Dist / MaxDistance, 0.1f, 1.f);
        Mesh->AddForce(DirToRod * ReelForce * Attenuation);
    }
}

void ALureActor::SetBeingReeled(bool bReeling, const FVector& ReelTargetIn)
{
    bIsBeingReeled = bReeling;
    ReelTarget = ReelTargetIn;

    if (bReeling)
    {
        // リール開始時に一瞬抵抗をリセット
        Mesh->SetLinearDamping(0.1f);
    }
}

void ALureActor::NotifyActorBeginOverlap(AActor* OtherActor)
{
    Super::NotifyActorBeginOverlap(OtherActor);

    AFishActor* Fish = Cast<AFishActor>(OtherActor);
    if (Fish)
    {
        Fish->StartFight(this);
        OnFishHit.Broadcast();
    }
}

void ALureActor::UpdateFishingLine(const FVector& Start, const FVector& End)
{
    // ここが空でもいい（実装だけしておく）
}