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
