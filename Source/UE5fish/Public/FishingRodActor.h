// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FishingRodActor.generated.h"

UCLASS()
class UE5FISH_API AFishingRodActor : public AActor
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Fishing, meta = (AllowPrivateAccess = "true"))
	UStaticMeshComponent* RodMesh;

public:
	AFishingRodActor();

	// 釣り状態
	bool bFishBiting = false;

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	

	// 糸テンション
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fishing")
	float RodTension = 50.0f;

	UPROPERTY(EditAnywhere, Category = "Fishing")
	float RodMin = 10.0f;

	UPROPERTY(EditAnywhere, Category = "Fishing")
	float RodMax = 90.0f;

	// プレイヤー入力
	float RodInput = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Fishing")
	float RodInputSpeed = 40.0f;

	// 魚の引き
	UPROPERTY(EditAnywhere, Category = "Fishing")
	float FishStrength = 20.0f;

	// 成功判定
	float CatchTime = 0.0f;
	UPROPERTY(EditAnywhere, Category = "Fishing")
	float MaxCatchTime = 5.0f;

	FTimerHandle BiteTimerHandle;

	void FishBite();
	void EndBite();

public:
	// プレイヤー入力
	void InputVertical(float Value); // W/S入力

	// 釣り開始
	void StartFishing();

	// 魚ゲット判定
	void ReelAttempt();

};
