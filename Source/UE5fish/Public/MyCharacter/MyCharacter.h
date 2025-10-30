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
	//�J�����̈ʒu
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	USpringArmComponent* CameraBoom;

	//���C���J����
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FollowCamera;

	//�}�b�s���O�R���e�L�X�g
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	//�ړ��A�N�V����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	//���_�A�N�V����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	//�C���^���N�g
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* BoatInteractAction;

	//�ނ��
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fishing, meta = (AllowPrivateAccess = "true"))
	AFishingRodActor* FishingRod;

	//�A�N�V����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input, meta = (AllowPrivateAccess = "true"))
	UInputAction* FishingAction;

	//�ƃA�N�V����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Fishing, meta = (AllowPrivateAccess = "true"))
	TSubclassOf<class AFishingRodActor> FishingRodClass;

	// �V�A�N�V����
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* StartCasting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* ReleaseCasting;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* StartReel;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = Input)
	UInputAction* StopReel;

	UPROPERTY(BlueprintReadWrite, Category = "Fishing")
	bool bFishOnLine = false; // �����������Ă��邩

	UPROPERTY(BlueprintReadWrite, Category = "Fishing")
	bool bReeling = false; // �����Ă��邩

	// ���݂̃e���V�����i���̒���j
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
	/// ����
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

	/** �ނ�Ƃ̑����^���[�؂�ւ� */
	void ToggleFishingRod(bool bEquip);

	/** �ނ�Ƒ������ */
	bool bRodEquipped = false;

	/** �ނ蒆���ǂ��� */
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
