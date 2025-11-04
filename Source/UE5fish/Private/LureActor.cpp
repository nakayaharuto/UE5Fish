#include "LureActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/GameplayStatics.h"

ALureActor::ALureActor()
{
    PrimaryActorTick.bCanEverTick = true;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetSimulatePhysics(false);
    Mesh->SetMobility(EComponentMobility::Movable);
    Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
}

void ALureActor::BeginPlay()
{
    Super::BeginPlay();
}

void ALureActor::LaunchLure(const FVector& InTarget, float InSpeed)
{
    StartLocation = GetActorLocation();
    TargetLocation = InTarget;
    Speed = InSpeed;
    bIsFlying = true;
    bHitWater = false;
    bFishHit = false;
}

void ALureActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    FVector CurrentPos = GetActorLocation();
    UE_LOG(LogTemp, Log, TEXT("Lure Position: X=%.2f Y=%.2f Z=%.2f"),
        CurrentPos.X, CurrentPos.Y, CurrentPos.Z);
    // 巻き取り中はALureActor側で移動させない
    if (!bIsFlying || bIsBeingReeled) return;

    FVector Dir = (TargetLocation - GetActorLocation()).GetSafeNormal();
    FVector NewPos = GetActorLocation() + Dir * Speed * DeltaTime;

    float DistanceTravelled = FVector::Dist(StartLocation, NewPos);
    if (DistanceTravelled >= MaxDistance)
    {
        SetActorLocation(TargetLocation);
        bIsFlying = false;
        return;
    }

    // 水判定
    if (!bHitWater && WaterActorClass)
    {
        TArray<AActor*> WaterActors;
        UGameplayStatics::GetAllActorsOfClass(GetWorld(), WaterActorClass, WaterActors);
        for (AActor* Water : WaterActors)
        {
            if (Water && GetActorLocation().Z <= Water->GetActorLocation().Z + 10.f)
            {
                bHitWater = true;
                bIsFlying = false;
                OnHitWater.Broadcast();
                break;
            }
        }
    }

    // 魚ヒット判定
    if (!bFishHit)
    {
        float Chance = FishHitChancePerSecond * DeltaTime;
        if (FMath::FRandRange(0.f, 100.f) < Chance)
        {
            bFishHit = true;
            OnFishHit.Broadcast();
        }
    }

    SetActorLocation(NewPos);
}
