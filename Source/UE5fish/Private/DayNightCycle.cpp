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

    // 1. 時間を進める
    // TimeSpeed = 60 のとき、現実の1秒でゲーム内の1分進む計算
    CurrentTime += (DeltaTime * TimeSpeed) / 3600.0f;

    // 24時間を超えたら 0 に戻す
    if (CurrentTime >= 24.0f)
    {
        CurrentTime -= 24.0f;
    }

    // 2. 太陽と月の回転・状態を更新
    UpdateSunRotation(DeltaTime);
}

void ADayNightCycle::UpdateSunRotation(float DeltaTime)
{
    if (!MainLight) return;
    UDirectionalLightComponent* LightComp = MainLight->GetComponentByClass<UDirectionalLightComponent>();
    if (!LightComp) return;

    // --- 1. SunPositionでの計算 (前述と同じ) ---
    int32 Hours = FMath::FloorToInt(CurrentTime);
    float TotalMinutes = (CurrentTime - Hours) * 60.0f;
    int32 Minutes = FMath::FloorToInt(TotalMinutes);
    int32 Seconds = FMath::FloorToInt((TotalMinutes - Minutes) * 60.0f);

    FSunPositionData SunData;
    // 緯度・経度などはとりあえず固定値（東京付近など）で設定
    // 実際にはこれらもUPROPERTYにすると便利です
    USunPositionFunctionLibrary::GetSunPosition(
        35.68f,   // 緯度 (Latitude)
        139.76f,  // 経度 (Longitude)
        9.0f,     // タイムゾーン (TimeZone)
        false,    // 夏時間 (IsDaylightSavingTime)
        2024,     // 年
        6,        // 月
        21,       // 日
        Hours,
        Minutes,
        Seconds,
        SunData
    );

    // --- 2. 太陽か月かの判定 ---
    bool bIsSunBelowHorizon = (SunData.Elevation < 0.0f);

    if (!bIsSunBelowHorizon)
    {
        // 【昼モード：太陽】
        FRotator SunRot(SunData.Elevation, SunData.Azimuth, 0.0f);
        MainLight->SetActorRotation(SunRot);

        LightComp->SetIntensity(SunIntensity);
        LightComp->SetLightColor(FLinearColor::White);

        // 太陽として空の色に影響を与える
        LightComp->SetAtmosphereSunLightIndex(0);
        // 夜モードでオフにした場合に備えてオンに戻す
        LightComp->SetCastShadows(true);
    }
    else
    {
        // 【夜モード：月】
        // 修正ポイント：太陽が「-10度」にいるなら、月は真反対の「+10度」に配置する
        // Azimuth（方位角）も180度ずらすことで、太陽が沈んだ場所の反対から月が昇ります
        FRotator MoonRot(-SunData.Elevation, SunData.Azimuth + 180.0f, 0.0f);
        MainLight->SetActorRotation(MoonRot);

        LightComp->SetIntensity(MoonIntensity);
        LightComp->SetLightColor(FLinearColor(MoonColor));

        // 重要：月として扱うためにIndexを1に変更
        // これによりSkyAtmosphereは「太陽は沈んだ」と判断し、空を暗く保ちます
        LightComp->SetAtmosphereSunLightIndex(1);

        // --- 3. SkyLightの更新（負荷対策版） ---
        // 毎フレームRecaptureすると激重になるため、例えば「分」が変わった時だけ更新
        static int32 LastMinute = -1;
        if (Minutes != LastMinute)
        {
            if (MySkyLight && MySkyLight->GetLightComponent())
            {
                MySkyLight->GetLightComponent()->RecaptureSky();
            }
            LastMinute = Minutes;
        }
    }
}

// -----------------------------------------------------
// 昼か？
// -----------------------------------------------------
bool ADayNightCycle::IsDay() const
{
    return CurrentTime >= DayStartHour && CurrentTime < NightStartHour;
}