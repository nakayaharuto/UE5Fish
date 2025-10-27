// Fill out your copyright notice in the Description page of Project Settings.


#include "BoatPawn.h"
#include "MyCharacter/MyCharacter.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "Components/InputComponent.h"

// Sets default values
ABoatPawn::ABoatPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BoatMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Boatmesh"));
	SetRootComponent(BoatMesh);

	BoatMesh->SetSimulatePhysics(true);
	BoatMesh->SetLinearDamping(0.5f);
	BoatMesh->SetAngularDamping(0.5f);

	//座席の位置を設定
	SeatPosition = CreateDefaultSubobject<USceneComponent>(TEXT("SeatPosition"));
	SeatPosition->SetupAttachment(RootComponent);
	SeatPosition->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 600.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = false; // Rotate the arm based on the controller
	CameraBoom->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));

	// Create a follow camera
	BoatCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	BoatCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	BoatCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm
	BoatCamera->SetFieldOfView(90.0f);

	CameraBoom->bEnableCameraLag = true;
	CameraBoom->CameraLagSpeed = 3.0f;
	CameraBoom->CameraRotationLagSpeed = 5.0f;

	bHasDriver = false;

	ForwardInput = 0.0f;
	TurnInput = 0.0f;

}

// Called when the game starts or when spawned
void ABoatPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

void ABoatPawn::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

// Called every frame
void ABoatPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!bHasDriver) return;

	// 現在の速度
	FVector Velocity = BoatMesh->GetPhysicsLinearVelocity();
	float Speed = Velocity.Size();

	// --- 推進力 ---
	FVector Forward = GetActorForwardVector();
	FVector Force = Forward * ForwardInput * EnginPower;
	BoatMesh->AddForce(Force);

	// --- スムーズな旋回（速度に比例）---
	if (Speed > 10.0f)
	{
		// スピードが速いほど旋回できる（0〜1）
		float TurnRatio = FMath::Clamp(Speed / 2000.0f, 0.0f, 1.0f);
		// 実際の回転量を計算
		float TurnAmount = TurnInput * TurnSpeed * TurnRatio * DeltaTime;

		// 現在角度
		FRotator CurrentRot = GetActorRotation();
		// 目標角度
		FRotator TargetRot = CurrentRot;
		TargetRot.Yaw += TurnAmount;

		// 滑らかに補間して回す
		FRotator SmoothedRot = FMath::RInterpTo(CurrentRot, TargetRot, DeltaTime, 3.0f);
		SetActorRotation(SmoothedRot);
	}

	// --- 抵抗 ---
	const float LinearDrag = 0.8f;
	FVector DragForce = -Velocity * LinearDrag;
	BoatMesh->AddForce(DragForce);

	// --- 浮力 ---
	ApplyBuoyancy();
}



// Called to bind functionality to input
void ABoatPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// BoatMoving
		EnhancedInputComponent->BindAction(BoatMoveAction, ETriggerEvent::Triggered, this, &ABoatPawn::BoatMove);

		// BoatLooking
		EnhancedInputComponent->BindAction(BoatLookAction, ETriggerEvent::Triggered, this, &ABoatPawn::BoatLook);

		//Exit
		EnhancedInputComponent->BindAction(ExitBoatAction, ETriggerEvent::Triggered, this, &ABoatPawn::ExitBoat);

	}

}

void ABoatPawn::BoatMove(const FInputActionValue& Value)
{
	if (!bHasDriver) return;
	// 入力値を FVector2D で取得
	FVector2D Movement = Value.Get<FVector2D>();
	// 前進後退入力
	ForwardInput = Movement.Y;
	// 左右旋回入力
	TurnInput = Movement.X;

	if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(GetRootComponent()))
	{
		// 前後力
		FVector Force = GetActorForwardVector() * ForwardInput * 200000.f;
		// 左右力（旋回は別でやる場合は加えない）
		Force += GetActorRightVector() * TurnInput * 100000.f;

		Mesh->AddForce(Force);

		// 水抵抗（速度の逆方向）
		FVector Velocity = Mesh->GetPhysicsLinearVelocity();
		const float DragCoefficient = 300.f;
		FVector DragForce = -Velocity * DragCoefficient;
		DragForce.Z = 0; // 浮力には影響しない
		Mesh->AddForce(DragForce);
	}
}

void ABoatPawn::BoatLook(const FInputActionValue& Value)
{
	if (!bHasDriver) return;
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ABoatPawn::ExitBoat(const FInputActionValue& Value)
{
	if (!bHasDriver) return;

	// プレイヤーコントローラを取得
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// OwnerにしていたCharacterを取得
	AMyCharacter* Character = Cast<AMyCharacter>(GetOwner());
	if (!Character) return;

	// 降りる位置を決定
	FVector ExitLoc = GetActorLocation() + GetActorRightVector() * 200.0f;
	Character->SetActorLocation(ExitLoc);
	Character->SetActorEnableCollision(true);

	// 所有権・操作を戻す
	PC->UnPossess();
	PC->Possess(Character);

	Character->bIsInBoat = false;
	Character->CurrentBoat = nullptr;
	bHasDriver = false;

	// カメラを戻す
	PC->SetViewTargetWithBlend(Character, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);


}

void ABoatPawn::ApplyBuoyancy()
{
	FVector BoatLocation = BoatMesh->GetComponentLocation();
	float WaterHeight = 0.0f;//海面Y座標
	float Depth = WaterHeight - BoatLocation.Z;

	if (Depth > 0)
	{
		// 深さに比例した浮力を加える
		FVector UpFroce = FVector(0.0f, 0.0f, Depth * BuoyancyForce);
		BoatMesh->AddForce(UpFroce);

		// 水の粘性抵抗
		FVector Velocity = BoatMesh->GetPhysicsLinearVelocity();
		FVector Drag = -Velocity * WaterDrag;
		BoatMesh->AddForce(Drag);

	}
}

