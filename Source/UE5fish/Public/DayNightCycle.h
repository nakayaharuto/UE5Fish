#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
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
    ADirectionalLight* SunLight;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "DayNight")
    ADirectionalLight* MoonLight;

private:
    float CurrentTime = 12.0f; // 12:00 スタート

    bool IsDay() const;
    void UpdateLightPriority();
    void UpdateSunRotation(float DeltaTime);
};