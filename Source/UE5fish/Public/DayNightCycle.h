// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DayNightCycle.generated.h"

class ADirectionalLight;

UCLASS()
class UE5FISH_API ADayNightCycle : public AActor
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "DayNight")
	ADirectionalLight* SunLight;	//太陽のライト

	UPROPERTY(EditAnywhere, Category = "DayNight")
	ADirectionalLight* MoonLight;	//月のライト

	UPROPERTY(EditAnywhere, Category = "DayNight")
	float DaySpeed = 0.1f;

	UPROPERTY(EditAnywhere, Category = "DayNight")
	float CurrentTime = 12.0f;

public:	
	// Sets default values for this actor's properties
	ADayNightCycle();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:
	void UpdateSunRotion();
	void UpdateMoonRotation();
	void UpdateLightIntensity();
};
