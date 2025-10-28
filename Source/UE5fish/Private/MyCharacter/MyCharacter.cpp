#include "MyCharacter/MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "FishingRodActor.h"
#include "BoatPawn.h"
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

	// 移動はプレイヤーの向きを変える
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;
}

void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();

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
}

//////////////////////////////////////////////////////////////////////////
// 入力処理

void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction) EnhancedInput->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyCharacter::Move);
		if (LookAction) EnhancedInput->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);
		if (BoatInteractAction) EnhancedInput->BindAction(BoatInteractAction, ETriggerEvent::Triggered, this, &AMyCharacter::InteractWithBoat);
		if (FishingAction) EnhancedInput->BindAction(FishingAction, ETriggerEvent::Triggered, this, &AMyCharacter::ToggleEquipRod);
		if (FishingMoveAction) EnhancedInput->BindAction(FishingMoveAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveFishingRod);
		if (CastLineAction) EnhancedInput->BindAction(CastLineAction, ETriggerEvent::Started, this, &AMyCharacter::StartCasting);
		if (CastLineAction) EnhancedInput->BindAction(CastLineAction, ETriggerEvent::Completed, this, &AMyCharacter::ReleaseCasting);
		if (ReelInAction) EnhancedInput->BindAction(ReelInAction, ETriggerEvent::Triggered, this, &AMyCharacter::ReelInLine);
		if (RodUpDownAction) EnhancedInput->BindAction(RodUpDownAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveRodUpDown);
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
// ボート関連（オプション）

void AMyCharacter::InteractWithBoat(const FInputActionValue& Value)
{
	if (CurrentBoat && !bIsInBoat)
	{
		AttachToActor(CurrentBoat, FAttachmentTransformRules::KeepWorldTransform);
		bIsInBoat = true;
	}
	else if (bIsInBoat)
	{
		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
		bIsInBoat = false;
	}
}

//////////////////////////////////////////////////////////////////////////
// 釣り関連

void AMyCharacter::ToggleEquipRod(const FInputActionValue& Value)
{
	bRodEquipped = !bRodEquipped;
	ToggleFishingRod(bRodEquipped);
}

void AMyCharacter::ToggleFishingRod(bool bEquip)
{
	if (!FishingRod) return;

	FishingRod->SetActorHiddenInGame(!bEquip);

	if (bEquip)
	{
		bIsFishing = true;

		// 🎣 カメラを右後ろにズラす
		CameraBoom->TargetArmLength = 350.0f;
		CameraBoom->SocketOffset = FVector(-50.f, 70.f, 20.f);
	}
	else
	{
		bIsFishing = false;

		// 通常カメラ位置に戻す
		CameraBoom->TargetArmLength = 300.0f;
		CameraBoom->SocketOffset = FVector::ZeroVector;
	}
}

// Cast 開始（クリック押し）
void AMyCharacter::StartCasting(const FInputActionValue& Value)
{
	if (!FishingRod || !bRodEquipped) return;
	FishingRod->BeginChargeCast();  // 投げ距離チャージ開始
}

// Cast 完了（クリック離す）
void AMyCharacter::ReleaseCasting(const FInputActionValue& Value)
{
	if (!FishingRod || !bRodEquipped) return;
	FishingRod->ReleaseCast();  // 投げる
}

// 巻き取り
void AMyCharacter::ReelInLine(const FInputActionValue& Value)
{
	if (!FishingRod || !bRodEquipped) return;
	FishingRod->ReelIn();  // 糸を引く
}

// 上下操作
void AMyCharacter::MoveRodUpDown(const FInputActionValue& Value)
{
	if (!FishingRod || !bRodEquipped) return;

	float Axis = Value.Get<float>();
	FishingRod->AdjustRodPitch(Axis);
}

void AMyCharacter::MoveFishingRod(const FInputActionValue& Value)
{
	if (!bIsFishing || !FishingRod) return;

	const float MoveValue = Value.Get<float>();
	FVector NewLoc = FishingRod->GetActorLocation();
	NewLoc += GetActorRightVector() * MoveValue * 10.0f;
	FishingRod->SetActorLocation(NewLoc);
}

void AMyCharacter::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->ClearAllMappings();
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}
}