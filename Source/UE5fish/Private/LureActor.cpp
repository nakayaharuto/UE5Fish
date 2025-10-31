#include "LureActor.h"
#include "Components/StaticMeshComponent.h"

ALureActor::ALureActor()
{
    PrimaryActorTick.bCanEverTick = false;

    // �X�P���^�����b�V���ɕύX
    Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetSimulatePhysics(true);
    Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
}

void ALureActor::BeginPlay()
{
    Super::BeginPlay();
}

void ALureActor::AddImpulse(const FVector& Force)
{
    if (Mesh)
    {
        Mesh->AddImpulse(Force, NAME_None, true);
    }
}
