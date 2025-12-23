#include "MyCharacter/MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "CaughtFish.h"
#include "FishingRodActor.h"
#include "FishingBattleWidget.h"
#include "GameInstance/FishingHUD.h" // Cast<AMyFishingHUD>用
#include "GameInstance/MyGameInstance.h" // Cast<UMyGameInstance>用
#include "LureActor.h"
#include "FishActor.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerController.h"
#include "Components/SkeletalMeshComponent.h"

//////////////////////////////////////////////////////////////////////////
// AMyCharacter

AMyCharacter::AMyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// カメラブーム（キャラクターの背後にカメラを保持）
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 300.0f;
	CameraBoom->bUsePawnControlRotation = true;

	// カメラ本体
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	FollowCamera->bUsePawnControlRotation = false;

	// 釣り用カメラ
	FishingCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FishingCamera"));
	FishingCamera->SetupAttachment(RootComponent);
	FishingCamera->SetRelativeLocation(FVector(-200.f, 0.f, 100.f));
	FishingCamera->bUsePawnControlRotation = true;
	FishingCamera->SetActive(false);


	// 移動はプレイヤーの向きを変える
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->bUseControllerDesiredRotation = false;
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		// 1. 入力モードをゲームのみに戻す
		FInputModeGameOnly InputMode;
		PC->SetInputMode(InputMode);

		// 2. マウスカーソルを消す
		PC->bShowMouseCursor = false;
	}

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	// 釣り竿のスポーン
	if (FishingRodClass)
	{
		FActorSpawnParameters SpawnParams;
		SpawnParams.Owner = this;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

		FishingRod = GetWorld()->SpawnActor<AFishingRodActor>(FishingRodClass, SpawnParams);
		if (FishingRod)
		{
			// ソケット名 "RodSocket" にアタッチ（メッシュに設定済み）
			FishingRod->AttachToComponent(GetMesh(),
				FAttachmentTransformRules::SnapToTargetNotIncludingScale,
				FName("RodSocket"));
			FishingRod->SetActorHiddenInGame(true);
		}
	}

	if (FishingRod)
	{
		//イベント
		FishingRod->OnStartFishBattle.AddDynamic(this, &AMyCharacter::ShowFishingUI);
		FishingRod->OnEndFishBattle.AddDynamic(this, &AMyCharacter::HideFishingUI);

		//魚ゲット UI イベントのバインド
		//FishingRod->OnFishCaughtUI.AddDynamic(this, &AMyCharacter::ShowCaughtFishWidget);
		
		FishingRod->OnFishCaughtUI.AddDynamic(this, &AMyCharacter::HandleFishCaught);
	}
}

//////////////////////////////////////////////////////////////////////////
// 入力処理

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		//移動・視点
		if (MoveAction) EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyCharacter::Move);
		if (LookAction) EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
		if (FishingAction) EnhancedInput->BindAction(FishingAction, ETriggerEvent::Triggered, this, &AMyCharacter::ToggleEquipRod);
		
		//インベントリ
		if (InventoryAction) EnhancedInput->BindAction(InventoryAction, ETriggerEvent::Started, this, &AMyCharacter::Inventory);

		//Eキーでキャスト
		if (StartCasting) EnhancedInput->BindAction(StartCasting, ETriggerEvent::Started, this, &AMyCharacter::StartCastingInput);
		//if (ReleaseCasting) EnhancedInput->BindAction(ReleaseCasting, ETriggerEvent::Started, this, &AMyCharacter::ReleaseCastingInput);
		
		//左クリックでゲージの調整
		if (StartReel) EnhancedInput->BindAction(StartReel, ETriggerEvent::Started, this, &AMyCharacter::StartReelInput);
		if (StopReel) EnhancedInput->BindAction(StopReel, ETriggerEvent::Completed, this, &AMyCharacter::StopReelInput);
	}
}

//////////////////////////////////////////////////////////////////////////
// 移動処理

void AMyCharacter::Move(const FInputActionValue& Value)
{
	if (bIsFishing) return; // 釣り中は移動禁止

	const FVector2D MovementVector = Value.Get<FVector2D>();
	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDir = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDir, MovementVector.Y);
		AddMovementInput(RightDir, MovementVector.X);
	}
}

void AMyCharacter::Look(const FInputActionValue& Value)
{
	const FVector2D LookAxisVector = Value.Get<FVector2D>();
	AddControllerYawInput(LookAxisVector.X);
	AddControllerPitchInput(LookAxisVector.Y);
}
//////////////////////////////////////////////////////////////////////////
// インベントリ関連

void AMyCharacter::Inventory(const FInputActionValue& Value)
{
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// 自分のHUDを取得してトグル関数を呼ぶだけ
	if (AFishingHUD* FishingHUD = Cast<AFishingHUD>(GetWorld()->GetFirstPlayerController()->GetHUD()))
	{
		UE_LOG(LogTemp, Warning, TEXT("iaiaiaiaiaia"));
		FishingHUD->ToggleFishAlbum();
	}
}


//////////////////////////////////////////////////////////////////////////
// 釣り関連

void AMyCharacter::ShowFishingUI()
{
	if (FishingBattleWidgetClass && !FishingBattleWidget)
	{
		FishingBattleWidget = CreateWidget<UFishingBattleWidget>(GetWorld(), FishingBattleWidgetClass);
		if (FishingBattleWidget)
			FishingBattleWidget->AddToViewport();
	}
}

void AMyCharacter::HideFishingUI(bool bSuccess)
{
	if (FishingBattleWidget)
	{
		FishingBattleWidget->RemoveFromParent();
		FishingBattleWidget = nullptr;
	}
}

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);


	// バトル中のみ UI更新
	if (bIsReelPressed && FishingRod && FishingRod->bIsFishBattle)
	{
		// DeltaTime を渡すことで、フレームレートに関係なく一定の速度でゲージが増える
		FishingRod->ReelProgress(DeltaTime);
	}

	// バトル中のみ UI更新
	if (FishingRod && FishingRod->bIsFishBattle && FishingBattleWidget)
	{
		float PlayerPct = FishingRod->PlayerGauge / FishingRod->GaugeMax;
		float FishPct = FishingRod->FishGauge / FishingRod->GaugeMax;

		FishingBattleWidget->UpdateGauges(PlayerPct, FishPct);
	}
}

void AMyCharacter::ToggleEquipRod(const FInputActionValue& Value)
{
	bRodEquipped = !bRodEquipped;
	ToggleFishingRod(bRodEquipped);
}

void AMyCharacter::ToggleFishingRod(bool bEquip)
{
	bIsFishing = !bIsFishing;
	if (FishingRod)
	{
		FishingRod->bEquipped = bEquip; // 竿アクターのbEquippedを更新

		if (bIsFishing)
		{
			// 🎣 釣り開始 → 初期化
			FishingRod->RodMesh->SetVisibility(true, true);
			FishingRod->ResetRodState();

		}
		else
		{
			// 🎣 釣り終了 → リセットして安全にしまう
			FishingRod->RodMesh->SetVisibility(false, true);
			FishingRod->ResetRodState();
		}
	}

	CameraBoom->SetActive(!bIsFishing);
	FollowCamera->SetActive(!bIsFishing);
	FishingCamera->SetActive(bIsFishing);
}

void AMyCharacter::StartCastingInput(const FInputActionValue& Value)
{
	if (!bIsFishing) return;

	//UE_LOG(LogTemp, Warning, TEXT("Character: StartCastingInput called. IsFishing: %d, Rod Valid: %d"), bIsFishing, (FishingRod != nullptr));

	FVector CamLoc = FishingCamera->GetComponentLocation();
	FVector CamDir = FishingCamera->GetForwardVector();

	FVector End = CamLoc + CamDir * 1500.f;
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, End, ECC_Visibility, Params))
		End = Hit.Location;

	FishingRod->CastToLocation(End);
}

void AMyCharacter::StartReelInput(const FInputActionValue& Value)
{
	bIsReelPressed = true;
}

void AMyCharacter::StopReelInput(const FInputActionValue& Value)
{
	bIsReelPressed = false;
	
}



void AMyCharacter::HandleFishCaught(FText FishName, float Size, UTexture2D* FishImage, int32 Rarity)
{
	APlayerController* PC = GetController<APlayerController>();
	if (!CaughtFishWidgetClass)
	{
		UE_LOG(LogTemp, Error, TEXT("UI Error: CaughtFishWidgetClass is NULL. (Blueprintで設定されていません)"));
	}
	if (!PC)
	{
		UE_LOG(LogTemp, Error, TEXT("UI Error: PlayerController (PC) is NULL."));
	}

	// 1. GameInstance を取得してキャスト
	if (UMyGameInstance* GI = Cast<UMyGameInstance>(GetGameInstance()))
	{
		// 2. システム側の図鑑に登録！
		GI->RegisterFishToAlbum(FishName.ToString(), Size, FishImage);
	}

	// 既にウィジェットが表示中の場合は、一度破棄してから再作成
	if (CurrentCaughtFishWidget && CurrentCaughtFishWidget->IsInViewport())
	{
		CurrentCaughtFishWidget->RemoveFromParent();
		CurrentCaughtFishWidget = nullptr;
	}

	// 2. ウィジェットの生成
	// CaughtFishWidgetClass は TSubclassOf<class UCaughtFish> で宣言されているはずです。
	CurrentCaughtFishWidget = CreateWidget<UCaughtFish>(PC, CaughtFishWidgetClass);

	if (CurrentCaughtFishWidget)
	{
		UE_LOG(LogTemp, Warning, TEXT("UI LOG: Successfully created CaughtFish widget."));

		// 3. データをウィジェットに設定
		CurrentCaughtFishWidget->SetFishData(FishName, Size, FishImage, Rarity);

		// 4. 画面に表示
		CurrentCaughtFishWidget->AddToViewport();

		// 5. 入力モードの切り替え (推奨)
		FInputModeUIOnly InputMode;
		InputMode.SetWidgetToFocus(CurrentCaughtFishWidget->TakeWidget());
		PC->SetInputMode(InputMode);
		PC->bShowMouseCursor = true;
	}
}

void AMyCharacter::CloseCaughtFishUI()
{
	APlayerController* PC = GetController<APlayerController>();

	if (CurrentCaughtFishWidget)
	{
		CurrentCaughtFishWidget->RemoveFromParent();
		CurrentCaughtFishWidget = nullptr;
	}

	if (PC)
	{
		// 入力モードをゲームプレイのみに戻す
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}
}

void AMyCharacter::PlayFishHitAnimation()
{
	if (FishHitMontage)
	{
		// アニメーションモンタージュを再生
		PlayAnimMontage(FishHitMontage);
		UE_LOG(LogTemp, Warning, TEXT("Character: Playing Fish Hit Montage!"));
	}
}