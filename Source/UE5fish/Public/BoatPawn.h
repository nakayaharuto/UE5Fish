// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "BoatPawn.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;

UCLASS()
class UE5FISH_API ABoatPawn : public APawn
{
	GENERATED_BODY()
public:
	//�{�[�g�̃��b�V��
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"));
	UStaticMeshComponent* BoatMesh;

	//�p�����[�^�̒����p
	UPROPERTY(EditAnywhere, Category = "Boat|Movement");
	float EnginPower = 50000.0f;

	UPROPERTY(EditAnywhere, Category = "Boat|Movement");
	float TurnSpeed = 20000.0f;

	//�����悻�̏d��
	UPROPERTY(EditAnywhere, Category = "Boat|Physics");
	float BuoyancyForce = 9800.0f;

	UPROPERTY(EditAnywhere, Category = "Boat|Physics");
	float WaterDrag = 0.3;

	UPROPERTY(EditAnywhere, Category = "Boat|Physics");
	float WaterHeight = 0.0f;

	//�J�����̈ʒu
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	//���C���J����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* BoatCamera;

	//�L����������Ă��邩
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Boat, meta = (AllowPrivateAccess = "true"))
	bool bHasDriver = false;

	//�}�b�s���O�R���e�L�X�g
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	//�L�����̍���ʒu
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Seat, meta = (AllowPrivateAccess = "true"))
	USceneComponent* SeatPosition;

	//�ړ��A�N�V����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BoatMoveAction;

	//���_�A�N�V����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BoatLookAction;

	//�C���^���N�g
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* ExitBoatAction;

	FVector CurrentVelocity;

	float ForwardInput;
	float TurnInput;

public:
	// Sets default values for this pawn's properties
	ABoatPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void BoatMove(const FInputActionValue& Value);

	void BoatLook(const FInputActionValue& Value);

	void ExitBoat(const FInputActionValue& Value);

	//���́E��R�v�Z
	void ApplyBuoyancy();

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	virtual void NotifyControllerChanged() override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
