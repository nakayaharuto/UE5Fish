// Fill out your copyright notice in the Description page of Project Settings.


#include "FishingRodActor.h"
#include "TimerManager.h"
#include "Kismet/KismetMathLibrary.h"
#include "Engine/World.h"

// Sets default values
AFishingRodActor::AFishingRodActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFishingRodActor::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFishingRodActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (!bFishBiting) return;

	// --- 1. 魚の引きをランダムで加算 ---
	float FishPull = UKismetMathLibrary::RandomFloatInRange(-FishStrength, FishStrength);
	RodTension += FishPull * DeltaTime;

	// --- 2. プレイヤー入力による補正 ---
	RodTension += RodInput * RodInputSpeed * DeltaTime;

	// --- 3. Clamp ---
	RodTension = FMath::Clamp(RodTension, 0.0f, 100.0f);

	// --- 4. 失敗判定 ---
	if (RodTension <= RodMin || RodTension >= RodMax)
	{
		bFishBiting = false;
		RodTension = 50.0f;
		CatchTime = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("❌ 糸切れ or 魚が逃げた"));
		return;
	}

	// --- 5. 成功判定 ---
	CatchTime += DeltaTime;
	if (CatchTime >= MaxCatchTime)
	{
		bFishBiting = false;
		RodTension = 50.0f;
		CatchTime = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("魚ゲット！"));
	}
}

void AFishingRodActor::StartFishing()
{
	if (bFishBiting) return;

	// ランダム秒後に魚が食いつく
	float WaitTime = UKismetMathLibrary::RandomFloatInRange(2.0f, 8.0f);
	GetWorldTimerManager().SetTimer(BiteTimerHandle, this, &AFishingRodActor::FishBite, WaitTime, false);
	UE_LOG(LogTemp, Log, TEXT("🎣 釣り開始。魚がヒットするまで %.1f 秒待機"), WaitTime);
}

void AFishingRodActor::FishBite()
{
	bFishBiting = true;
	UE_LOG(LogTemp, Warning, TEXT("🐟 魚が食いついた！ WSでテンションを維持せよ"));

	// 食いつき終了タイマー（強制失敗防止）
	GetWorldTimerManager().SetTimer(BiteTimerHandle, this, &AFishingRodActor::EndBite, 10.0f, false);
}

void AFishingRodActor::EndBite()
{
	if (bFishBiting)
	{
		bFishBiting = false;
		RodTension = 50.0f;
		CatchTime = 0.0f;
		UE_LOG(LogTemp, Warning, TEXT("🐟 魚が逃げた..."));
	}
}

void AFishingRodActor::InputVertical(float Value)
{
	RodInput = Value; // W/S入力 (-1~1)
}

void AFishingRodActor::ReelAttempt()
{
	// 任意の追加演出用。今回はTickで自動判定なので空でもOK
}