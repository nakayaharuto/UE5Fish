#include "LureActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

ALureActor::ALureActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;

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

        if (Dist < 50.f)
        {
            FVector SnapPos = ReelTarget - DirToRod.GetSafeNormal() * 10.f;
            Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
            SetActorLocation(SnapPos);
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