// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCharacter/MyCharacter.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SceneComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "Kismet/GameplayStatics.h"
#include "BoatPawn.h"

// Sets default values
AMyCharacter::AMyCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller


	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

}

void AMyCharacter::NotifyControllerChanged()
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

// Called when the game starts or when spawned
void AMyCharacter::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
//void AMyCharacter::Tick(float DeltaTime)
//{
//	Super::Tick(DeltaTime);
//
//}

// Called to bind functionality to input
void AMyCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {

		// Jumping
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		//EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AMyCharacter::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AMyCharacter::Look);

		//Interact
		EnhancedInputComponent->BindAction(InteractAction, ETriggerEvent::Triggered, this, &AMyCharacter::Interact);

		//Exit
		//EnhancedInputComponent->BindAction(ExitBoatAction, ETriggerEvent::Triggered, this, &AMyCharacter::ExitBoat);

	}
	
}

void AMyCharacter::Move(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

		// get right vector 
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		// add movement 
		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMyCharacter::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AMyCharacter::Interact(const FInputActionValue& Value)
{
	FVector MyLoc = GetActorLocation();

	// 近くのボートを探す
	TArray<AActor*> FoundBoats;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), ABoatPawn::StaticClass(), FoundBoats);

	for (AActor* Boat : FoundBoats)
	{
		float Dist = FVector::Dist(MyLoc, Boat->GetActorLocation());
		if (Dist < 300.0f)
		{
			ABoatPawn* BoatPawn = Cast<ABoatPawn>(Boat);
			if (BoatPawn && !BoatPawn->bHasDriver)
			{
				APlayerController* PC = Cast<APlayerController>(GetController());
				if (PC)
				{
					PC->Possess(BoatPawn);
					BoatPawn->bHasDriver = true;
					BoatPawn->SetOwner(this);
					CurrentBoat = BoatPawn;
					bIsInBoat = true;

					// キャラを座席にアタッチ（運転席）
					AttachToComponent(BoatPawn->SeatPosition, FAttachmentTransformRules::SnapToTargetNotIncludingScale);
					SetActorHiddenInGame(false);
					SetActorEnableCollision(false);

					if (BoatPawn->BoatCamera)
					{
						PC->SetViewTargetWithBlend(BoatPawn, 0.5f, EViewTargetBlendFunction::VTBlend_Cubic);
					}
					
				}
				break;
			}
		}
	}
}

//void AMyCharacter::ExitBoat(const FInputActionValue& Value)
//{
//	if (!bIsInBoat || !CurrentBoat) return;
//
//	APlayerController* PC = Cast<APlayerController>(CurrentBoat->GetController());
//	if (PC)
//	{
//		CurrentBoat->bHasDriver = false;
//		DetachFromActor(FDetachmentTransformRules::KeepWorldTransform);
//
//		FVector ExitLoc = CurrentBoat->GetActorLocation() + CurrentBoat->GetActorRightVector() * 200.0f;
//		SetActorLocation(ExitLoc);
//		SetActorEnableCollision(true);
//
//		PC->UnPossess();
//		PC->Possess(this);
//
//		bIsInBoat = false;
//		CurrentBoat = nullptr;
//	}
//}
