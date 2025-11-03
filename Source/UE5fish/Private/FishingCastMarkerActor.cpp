// FishingCastMarkerActor.cpp
#include "FishingCastMarkerActor.h"
#include "NiagaraComponent.h"

AFishingCastMarkerActor::AFishingCastMarkerActor()
{
    PrimaryActorTick.bCanEverTick = false;
    MarkerFX = CreateDefaultSubobject<UNiagaraComponent>(TEXT("MarkerFX"));
    RootComponent = MarkerFX;
}
