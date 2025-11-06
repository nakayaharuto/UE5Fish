#include "FishActor.h"
#include "Components/StaticMeshComponent.h"
#include "Kismet/KismetMathLibrary.h"

AFishActor::AFishActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetLifeSpan(6.f); // 6秒で自動消去（必要なければ削除）
}

void AFishActor::BeginPlay()
{
    Super::BeginPlay();

    FVector Impulse = UKismetMathLibrary::RandomUnitVector() * 150.f + FVector(0, 0, 250.f);
    Mesh->AddImpulse(Impulse);
}

void AFishActor::StartFight(AActor* TargetLure)
{
    bIsFighting = true;
    LureTraget = TargetLure;
}

void AFishActor::StopFight()
{
    bIsFighting = false;
    LureTraget = nullptr;
}

void AFishActor::Tick(float DeletaTime)
{
    Super::Tick(DeletaTime);

    if (!bIsFighting || !LureTraget)return;

    FightTimer += DeletaTime;

    // 一定周期で方向をランダムに変える
    if (FightTimer > 1.2f)
    {
        FVector RandomDir = UKismetMathLibrary::RandomUnitVector();
        RandomDir.Z = 0.f; // 水平方向中心
        FVector Force = RandomDir * 1500.f;

        Mesh->AddForce(Force);
        FightTimer = 0.f;
    }
}
