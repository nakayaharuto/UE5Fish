#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/DirectionalLightComponent.h"
#include "SunPosition.h"
#include "Engine/SkyLight.h"
#include "Components/SkyLightComponent.h"
#include "DayNightCycle.generated.h"

class ADirectionalLight;



UCLASS()
class UE5FISH_API ADayNightCycle : public AActor
{
    GENERATED_BODY()

public:
    ADayNightCycle();

protected:
    virtual void BeginPlay() override;

public:
    virtual void Tick(float DeltaTime) override;

    // --------------------------------------
    // 設定項目（BPで編集可能）
    // --------------------------------------
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float DayStartHour = 6.0f;      // 昼の開始（6時）

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float NightStartHour = 18.0f;   // 夜の開始（18時）

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float TimeSpeed = 60.0f; // 1秒 = 1分（ゲーム内）

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float SunIntensity = 10.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    float MoonIntensity = 0.5f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    FColor MoonColor = FColor(200, 230, 255); // 少し青白い

    // MoonLight は削除し、1つの SunLight を使い回す
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    ADirectionalLight* MainLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    ASkyLight* MySkyLight;


private:
    float CurrentTime = 12.0f; // 12:00 スタート

    bool IsDay() const;
    void UpdateLightPriority();
    void UpdateSunRotation(float DeltaTime);
};