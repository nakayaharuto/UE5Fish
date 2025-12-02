#include "MyCharacter/MyCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "FishingRodActor.h"
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
		if (FishingAction) EnhancedInput->BindAction(FishingAction, ETriggerEvent::Triggered, this, &AMyCharacter::ToggleEquipRod);
		if (StartCasting) EnhancedInput->BindAction(StartCasting, ETriggerEvent::Triggered, this, &AMyCharacter::StartCastingInput);
		if (ReleaseCasting) EnhancedInput->BindAction(ReleaseCasting, ETriggerEvent::Started, this, &AMyCharacter::ReleaseCastingInput);
		if (StartReel) EnhancedInput->BindAction(StartReel, ETriggerEvent::Triggered, this, &AMyCharacter::StartReelInput);
		if (StopReel) EnhancedInput->BindAction(StopReel, ETriggerEvent::Completed, this, &AMyCharacter::StopReelInput);
		//if (RodUpDownAction) EnhancedInput->BindAction(RodUpDownAction, ETriggerEvent::Triggered, this, &AMyCharacter::MoveRodUpDown);
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
// 釣り関連

void AMyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bIsReelingFish && LureActor)
	{
		// 魚の方向
		FVector FishDir = (LureActor->GetActorLocation() - GetActorLocation()).GetSafeNormal();

		// プレイヤーがスティックを倒した方向を取得
		FVector2D StickInput = CurrentInputDirection; // ← InputAxis で更新
		FVector PlayerTiltDir = FVector(StickInput.X, StickInput.Y, 0.f).GetSafeNormal();

		// 竿を傾ける
		if (!PlayerTiltDir.IsNearlyZero())
		{
			// 竿の回転制御
			FRotator TargetRot = FRotationMatrix::MakeFromX(PlayerTiltDir * -1).Rotator();
			FishingRod->SetActorRotation(FMath::RInterpTo(FishingRod->GetActorRotation(), TargetRot, DeltaTime, 3.f));
		}

		// 魚との張力を物理的に再現
		FVector TensionDir = (GetActorLocation() - LureActor->GetActorLocation()).GetSafeNormal();
		float TensionStrength = FVector::DotProduct(TensionDir, -FishDir); // 魚と逆向きなら強い
		LureActor->Mesh->AddForce(TensionDir * TensionStrength * 2500.f);
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
	if (!bIsFishing || !FishingRod) return;

	FVector CamLoc = FishingCamera->GetComponentLocation();
	FVector CamDir = FishingCamera->GetForwardVector();

	FVector End = CamLoc + CamDir * 1500.f;
	FHitResult Hit;
	FCollisionQueryParams Params;
	Params.AddIgnoredActor(this);

	if (GetWorld()->LineTraceSingleByChannel(Hit, CamLoc, End, ECC_Visibility, Params))
		End = Hit.Location;

	//FishingRod->ShowCastTarget(End);
}

void AMyCharacter::ReleaseCastingInput(const FInputActionValue& Value)
{
	if (!bIsFishing || !FishingRod) return;

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
	if (FishingRod)
		FishingRod->StartReel();
}

void AMyCharacter::StopReelInput(const FInputActionValue& Value)
{
	if (FishingRod)
		FishingRod->StopReel();
}
