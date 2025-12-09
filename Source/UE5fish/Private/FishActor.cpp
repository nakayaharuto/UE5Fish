#include "FishActor.h"
#include "Components/StaticMeshComponent.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"

AFishActor::AFishActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    SetLifeSpan(6.f); // 6•b‚ÅŽ©“®Á‹Ži•K—v‚È‚¯‚ê‚Îíœj
}

void AFishActor::BeginPlay()
{
    Super::BeginPlay();

}


void AFishActor::ShowFish()
{
    Mesh->SetVisibility(true);

    //3•bŒã”ñ•\Ž¦‚Ö
    GetWorldTimerManager().SetTimer(HideTimerHandle, this, &AFishActor::HideFish, 3.f, false);
}

float AFishActor::GetCurrentDynamicResistance(float CurrentFishGauge, float GaugeMax) const
{
    // ’ïR—Í = Šî–{’ïR + ƒQ[ƒWŠ„‡ * Å‘å‘‰ÁŒW”
    float DynamicResistance = BaseResistance + (CurrentFishGauge / GaugeMax) * MaxResistanceMultiplier;
    return DynamicResistance;
}

void AFishActor::HideFish()
{
    Mesh->SetVisibility(false);
}


void AFishActor::Tick(float DeletaTime)
{
    Super::Tick(DeletaTime);

}
