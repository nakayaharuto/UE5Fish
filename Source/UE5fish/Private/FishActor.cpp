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
    // ‹›ƒQ[ƒW‚Ìis“x (0.0 ‚©‚ç 1.0)
    float ProgressRatio = CurrentFishGauge / GaugeMax;

    // ’ïR—Í‚ÍA‹›ƒQ[ƒW‚ª’á‚¢‚Ù‚Ç‚‚­‚È‚éiŒ³‹C‚È‚Ù‚Ç’ïR‚·‚éj
    // is“x‚ª‚‚­‚È‚é‚ÆA’ïR—Í‚Í Max ‚©‚ç Base ‚ÉŒü‚©‚Á‚ÄŒ¸­‚·‚éB
    float ResistanceRatio = 1.0f - ProgressRatio; // 0% ‚Å 1.0, 100% ‚Å 0.0

    // ’ïR—Í‚Ì•Ï“®•
    float ResistanceRange = MaxResistanceMultiplier - BaseResistance;

    // Œ»Ý‚Ì“®“I’ïR—Í = ƒx[ƒX’ïR—Í + (•Ï“®• * ’ïR”ä—¦)
    // FishGauge‚ª’á‚¢‚Ù‚Ç (ResistanceRatio‚ª‘å‚«‚¢‚Ù‚Ç) ’ïR—Í‚ª MaxResistanceMultiplier ‚É‹ß‚Ã‚­
    float DynamicResistance = BaseResistance + (ResistanceRange * ResistanceRatio);

    // ’ïR—Í‚Íí‚É³‚Ì’l‚Å‚ ‚é‚±‚Æ‚ð•ÛØ
    return FMath::Max(DynamicResistance, 0.1f);
}

void AFishActor::HideFish()
{
    Mesh->SetVisibility(false);
}


void AFishActor::Tick(float DeletaTime)
{
    Super::Tick(DeletaTime);

}

void AFishActor::SetFishData(const FString& Name, float Size, int32 InRarity,  float PlayerGaugeDecay, float InBaseResistance,
    float InMaxResistanceMultiplier, UTexture2D* InTexture)
{

    FishName = Name;
    SizeCm = Size;
    Rarity = InRarity;
    this->UITexture = InTexture; // UI—p‚Ì‰æ‘œ‚ð‚±‚±‚ÅŽó‚¯Žæ‚é
}