#include "LureActor.h"
#include "Components/StaticMeshComponent.h"

ALureActor::ALureActor()
{
    PrimaryActorTick.bCanEverTick = false;

    Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
    RootComponent = Mesh;
    Mesh->SetSimulatePhysics(true);
    Mesh->SetCollisionProfileName(TEXT("PhysicsActor"));
    Mesh->SetNotifyRigidBodyCollision(true); // ← ヒットイベントを受け取るため
    Mesh->OnComponentHit.AddDynamic(this, &ALureActor::OnHit);

    AttachPoint = CreateDefaultSubobject<USceneComponent>(TEXT("AttachPoint"));
    AttachPoint->SetupAttachment(Mesh);
    AttachPoint->SetRelativeLocation(FVector(0.f, 0.f, 25.f));

    Mesh->SetLinearDamping(1.0f);
    Mesh->SetAngularDamping(2.0f);
}

void ALureActor::BeginPlay()
{
    Super::BeginPlay();
    if (Mesh)
    {
        // メッシュをムーバブルに変更
        Mesh->SetMobility(EComponentMobility::Movable);

        // 物理を有効化
        Mesh->SetSimulatePhysics(true);

        // 重力を有効に
        Mesh->SetEnableGravity(true);
    }
    
}

void ALureActor::AddImpulse(const FVector& Force)
{
    if (Mesh && !bHasStopped)
    {
        Mesh->AddImpulse(Force, NAME_None, true);
    }
}

void ALureActor::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor,
    UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
    if (bHasStopped) return;

    if (OtherActor && OtherActor != this)
    {
        bHasStopped = true;

        // 速度を完全に停止
        Mesh->SetPhysicsLinearVelocity(FVector::ZeroVector);
        Mesh->SetPhysicsAngularVelocityInDegrees(FVector::ZeroVector);

        // 一瞬遅らせて物理停止（即時止めるとヒット直後の力が残るため）
        FTimerHandle TimerHandle_StopPhysics;
        GetWorld()->GetTimerManager().SetTimer(TimerHandle_StopPhysics, [this]()
            {
                if (Mesh)
                {
                    Mesh->SetSimulatePhysics(false);
                    Mesh->SetEnableGravity(false);
                }

            }, 0.05f, false);

        // 少し沈むように位置を調整
        FVector Loc = GetActorLocation();
        SetActorLocation(Loc + FVector(0.f, 0.f, -3.f));

        UE_LOG(LogTemp, Log, TEXT("Lure stopped at %s"), *Loc.ToString());
    }
}
