#include "DayNightCycle.h"
#include "Engine/DirectionalLight.h"
#include "Components/DirectionalLightComponent.h"

ADayNightCycle::ADayNightCycle()
{
    PrimaryActorTick.bCanEverTick = true;
}

void ADayNightCycle::BeginPlay()
{
    Super::BeginPlay();
}

void ADayNightCycle::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // ゲーム内時間を進める
    CurrentTime += DeltaTime * TimeSpeed / 60.0f;
    if (CurrentTime >= 24.0f) CurrentTime -= 24.0f;

    // ライトの影優先度切り替え
    UpdateLightPriority();

    // 太陽を回す
    UpdateSunRotation(DeltaTime);
}

// -----------------------------------------------------
// 昼か？
// -----------------------------------------------------
bool ADayNightCycle::IsDay() const
{
    return (CurrentTime >= DayStartHour && CurrentTime < NightStartHour);
}

// -----------------------------------------------------
// ForwardShadingPriority の切り替え（影の主ライト）
// -----------------------------------------------------
void ADayNightCycle::UpdateLightPriority()
{
    if (!SunLight || !MoonLight) return;

    auto* SunComp = Cast<UDirectionalLightComponent>(SunLight->GetLightComponent());
    auto* MoonComp = Cast<UDirectionalLightComponent>(MoonLight->GetLightComponent());

    if (!SunComp || !MoonComp) return;

    if (IsDay())
    {
        SunComp->ForwardShadingPriority = 10;
        MoonComp->ForwardShadingPriority = 1;
    }
    else
    {
        SunComp->ForwardShadingPriority = 1;
        MoonComp->ForwardShadingPriority = 10;
    }
}
// -----------------------------------------------------
// 太陽光の角度（太陽の見た目が動く）
// -----------------------------------------------------
void ADayNightCycle::UpdateSunRotation(float DeltaTime)
{
    if (!SunLight) return;

    float SunPitch = (CurrentTime / 24.0f) * 360.0f - 90.0f;

    FRotator NewRot = FRotator(SunPitch, 0.0f, 0.0f);
    SunLight->SetActorRotation(NewRot);
}