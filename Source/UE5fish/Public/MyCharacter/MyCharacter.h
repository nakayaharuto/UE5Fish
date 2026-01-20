// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "CaughtFish.h"
#include "MyCharacter.generated.h"


class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UFishingBattleWidget;
class UInputAction;
class AFishActor;
class ALureActor;
struct FInputActionValue;
class AFishingRodActor;
class UCaughtFish;


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

	//インベントリ
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* InventoryAction;

	//アクション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FishingAction;

	//竿アクション
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fishing, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AFishingRodActor> FishingRodClass;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Fishing")
	FVector2D CurrentInputDirection;

	//Widge

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "UI")
	TSubclassOf<class UFishingBattleWidget> FishingBattleWidgetClass;

	UPROPERTY()
	UFishingBattleWidget* FishingBattleWidget;

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

	//フィッシュカメラ
	UPROPERTY(VisibleAnywhere)
	UCameraComponent* FishingCamera;

	UFUNCTION()
	void ShowFishingUI();

	UFUNCTION()
	void HideFishingUI(bool bSuccess);

	// UIウィジェットのクラスリファレンス

	UPROPERTY(EditDefaultsOnly, Category = "Fishing UI")
	TSubclassOf<class UCaughtFish> CaughtFishWidgetClass;

	// 釣果結果を表示するウィジェットのインスタンス
	UPROPERTY()
	UCaughtFish* CurrentCaughtFishWidget;
	// 釣果イベントを受け取る関数
	UFUNCTION() 
		void HandleFishCaught(FText FishName, float Size, UTexture2D* FishImage);

	UFUNCTION(BlueprintCallable, Category = "Fishing UI")
	void CloseCaughtFishUI();

	//釣り竿を取り出した時
	void StartFishAnimation();

	// 魚がヒットした時に呼ばれる関数
	void PlayFishHitAnimation();

	UFUNCTION(BlueprintCallable, Category = "Fishing")
	void ResetCatchFlag() { bHasProcessedCatch = false; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
	UAnimMontage* FishHitMontage;

public:
	// Sets default values for this character's properties
	AMyCharacter();
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

protected:
	//釣り竿
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fishing, meta = (AllowPrivateAccess = "true"))
	AFishingRodActor* FishingRod;

	//ルアー
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fishing, meta = (AllowPrivateAccess = "true"))
	ALureActor* LureActor;

	
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Animations")
	UAnimMontage* StartFishMontage;

	/// <summary>
	/// 入力
	/// </summary>
	/// <param name="Value"></param>
	void Move(const FInputActionValue& Value);//移動
	void Look(const FInputActionValue& Value);//視点
	void Inventory(const FInputActionValue& Value);//視点
	void ToggleEquipRod(const FInputActionValue& Value);
	void StartCastingInput(const FInputActionValue& Value);
	void StartReelInput(const FInputActionValue& Value);
	void StopReelInput(const FInputActionValue& Value);

	/** 釣り竿の装備／収納切り替え */
	void ToggleFishingRod(bool bEquip);

	/** 釣り竿装備状態 */
	bool bRodEquipped = false;

	/** 釣り中かどうか */
	bool bIsFishing = false;

	/** 状態が長押しか */
	bool bIsReelPressed = false;

public:	
	bool bIsReelingFish = false;
protected:

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	UPROPERTY()
	bool bHasProcessedCatch = false;
public:
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	FORCEINLINE class UCameraComponent* GetFollowCamera() const { return FollowCamera; }

public:

	bool bIsInBoat = false;
	class ABoatPawn* CurrentBoat = nullptr;
};
