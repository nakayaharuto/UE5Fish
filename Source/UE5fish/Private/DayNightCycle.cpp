// Fill out your copyright notice in the Description page of Project Settings.


#include "DayNightCycle.h"
#include "Engine/DirectionalLight.h"
#include "Components/LightComponent.h"

// Sets default values
ADayNightCycle::ADayNightCycle()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ADayNightCycle::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ADayNightCycle::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 時間を進める
	CurrentTime += DeltaTime * DaySpeed;
	if (CurrentTime >= 24.0f)
	{
		CurrentTime -= 24.0f;
	}

	// 太陽の回転を更新
	UpdateSunRotion();
	UpdateMoonRotation();
	UpdateLightIntensity();
}

//太陽の回転
void ADayNightCycle::UpdateSunRotion()
{
	if (!SunLight) return;

	// 時間を角度に変換（0〜24 → -90〜270度）
	float SunPitch = (CurrentTime / 24.0f) * 360.0f - 90.0f;
	FRotator NewRotation(SunPitch, 0.0f, 0.0f);

	SunLight->SetActorRotation(NewRotation);

	// 昼夜で光の強さを変える（任意）
	if (UActorComponent* Comp = SunLight->GetComponentByClass(ULightComponent::StaticClass()))
	{
		ULightComponent* LightComp = Cast<ULightComponent>(Comp);
		if (LightComp)
		{
			if (CurrentTime < 6.0f || CurrentTime > 18.0f)
				LightComp->SetIntensity(0.2f);  // 夜
			else
				LightComp->SetIntensity(10.0f); // 昼
		}
	}
}

//月の回転
void ADayNightCycle::UpdateMoonRotation()
{
	if (!MoonLight)return;

	//太陽と逆方向に動かす
	float MoonPitch = ((CurrentTime + 12.0f) / 24.0f) * 360.0f - 90.0f;
	FRotator NewRotation(MoonPitch, 0.0f, 0.0f);
	MoonLight->SetActorRotation(NewRotation);
}

// 昼夜でライトのON/OFFと強度を変える
void ADayNightCycle::UpdateLightIntensity()
{
	if (!SunLight || !MoonLight) return;

	UActorComponent* SunComp = SunLight->GetComponentByClass(ULightComponent::StaticClass());
	UActorComponent* MoonComp = MoonLight->GetComponentByClass(ULightComponent::StaticClass());
	if (!SunComp || !MoonComp) return;

	ULightComponent* SunLightComp = Cast<ULightComponent>(SunComp);
	ULightComponent* MoonLightComp = Cast<ULightComponent>(MoonComp);

	if (CurrentTime >= 6.0f && CurrentTime <= 18.0f)
	{
		// 昼
		SunLightComp->SetIntensity(10.0f);
		MoonLightComp->SetIntensity(0.0f);
	}
	else
	{
		// 夜
		SunLightComp->SetIntensity(0.0f);
		MoonLightComp->SetIntensity(2.0f); // 月の光は弱め
	}
}