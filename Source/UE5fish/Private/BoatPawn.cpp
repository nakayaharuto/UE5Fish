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

	//���Ȃ̈ʒu��ݒ�
	SeatPosition = CreateDefaultSubobject<USceneComponent>(TEXT("SeatPosition"));
	SeatPosition->SetupAttachment(RootComponent);
	SeatPosition->SetRelativeLocation(FVector(0.0f, 0.0f, 80.0f));

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller


	// Create a follow camera
	BoatCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	BoatCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	BoatCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

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

	FVector Force = BoatMesh->GetForwardVector() * ForwardInput * EnginPower;
	BoatMesh->AddForce(Force);

	FVector Torque = FVector(0.0f, 0.0f, TurnInput * TurnSpeed);
	BoatMesh->AddTorqueInRadians(Torque);

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
	// ���͒l�� FVector2D �Ŏ擾
	FVector2D Movement = Value.Get<FVector2D>();
	float BoatForwardInput = Movement.Y; // W/S
	float BoatRightInput = Movement.X; // A/D

	if (UStaticMeshComponent* Mesh = Cast<UStaticMeshComponent>(GetRootComponent()))
	{
		// �O���
		FVector Force = GetActorForwardVector() * BoatForwardInput * 200000.f;
		// ���E�́i����͕ʂł��ꍇ�͉����Ȃ��j
		Force += GetActorRightVector() * BoatRightInput * 100000.f;

		Mesh->AddForce(Force);

		// ����R�i���x�̋t�����j
		FVector Velocity = Mesh->GetPhysicsLinearVelocity();
		const float DragCoefficient = 300.f;
		FVector DragForce = -Velocity * DragCoefficient;
		DragForce.Z = 0; // ���͂ɂ͉e�����Ȃ�
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

	// �v���C���[�R���g���[�����擾
	APlayerController* PC = Cast<APlayerController>(GetController());
	if (!PC) return;

	// Owner�ɂ��Ă���Character���擾
	AMyCharacter* Character = Cast<AMyCharacter>(GetOwner());
	if (!Character) return;

	// �~���ʒu������
	FVector ExitLoc = GetActorLocation() + GetActorRightVector() * 200.0f;
	Character->SetActorLocation(ExitLoc);
	Character->SetActorEnableCollision(true);

	// ���L���E�����߂�
	PC->UnPossess();
	PC->Possess(Character);

	Character->bIsInBoat = false;
	Character->CurrentBoat = nullptr;
	bHasDriver = false;

	// �J������߂�
	PC->SetViewTargetWithBlend(Character, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);


}

void ABoatPawn::ApplyBuoyancy()
{
	FVector BoatLocation = BoatMesh->GetComponentLocation();
	float WaterHeight = 0.0f;//�C��Y���W
	float Depth = WaterHeight - BoatLocation.Z;

	if (Depth > 0)
	{
		FVector UpFroce = FVector(0.0f, 0.0f, Depth * BuoyancyForce);
		BoatMesh->AddForce(UpFroce);

		FVector Velocity = BoatMesh->GetPhysicsLinearVelocity();
		FVector Drag = -Velocity * WaterDrag;
		BoatMesh->AddForce(Drag);

	}
}

