// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "MyCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class AFishingRodActor;


UCLASS()
class UE5FISH_API AMyCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	//カメラの位置
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	//メインカメラ
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	//マッピングコンテキスト
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	//移動アクション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	//視点アクション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	//インタラクト
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BoatInteractAction;

	//釣り竿
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fishing, meta = (AllowPrivateAccess = "true"))
	AFishingRodActor* FishingRod;

	//アクション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FishingAction;

	//竿アクション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fishing, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AFishingRodActor> FishingRodClass;

	// 新アクション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* StartCasting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ReleaseCasting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* StartReel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* StopReel;

	UPROPERTY(BlueprintReadWrite, Category = "Fishing")
	bool bFishOnLine = false; // 魚がかかっているか

	UPROPERTY(BlueprintReadWrite, Category = "Fishing")
	bool bReeling = false; // 巻いているか

	// 現在のテンション（糸の張り）
	UPROPERTY(BlueprintReadWrite, Category = "Fishing")
	float LineTension = 0.f;

public:
	// Sets default values for this character's properties
	AMyCharacter();
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

protected:
	// Called when the game starts or when spawned


	/// <summary>
	/// 入力
	/// </summary>
	/// <param name="Value"></param>
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void InteractWithBoat(const FInputActionValue& Value);
	void ToggleEquipRod(const FInputActionValue& Value);
	void StartCastingInput(const FInputActionValue& Value);
	void ReleaseCastingInput(const FInputActionValue& Value);
	void StartReelInput(const FInputActionValue& Value);
	void StopReelInput(const FInputActionValue& Value);

	/** 釣り竿の装備／収納切り替え */
	void ToggleFishingRod(bool bEquip);

	/** 釣り竿装備状態 */
	bool bRodEquipped = false;

	/** 釣り中かどうか */
	bool bIsFishing = false;

public:	
	// Called every frame
	//virtual void Tick(float DeltaTime) override;
protected:

	//virtual void NotifyControllerChanged() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:

	bool bIsInBoat = false;
	class ABoatPawn* CurrentBoat = nullptr;
};
