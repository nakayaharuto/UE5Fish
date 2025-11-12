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
    SetLifeSpan(6.f); // 6秒で自動消去（必要なければ削除）
}

void AFishActor::BeginPlay()
{
    Super::BeginPlay();

}


void AFishActor::ShowFish()
{
    Mesh->SetVisibility(true);

    //3秒後非表示へ
    GetWorldTimerManager().SetTimer(HideTimerHandle, this, &AFishActor::HideFish, 3.f, false);
}

void AFishActor::HideFish()
{
    Mesh->SetVisibility(false);
}


void AFishActor::Tick(float DeletaTime)
{
    Super::Tick(DeletaTime);

}
