// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "GameFramework/Character.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerInput.h"

#include "FPController.generated.h"

USTRUCT()
struct FCameraShakes {
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditInstanceOnly, meta = (ToolTip = "Idle Camera Shake"), Category = "Shakes")
		TSubclassOf<class UCameraShakeBase> IdleShake;
	UPROPERTY(EditInstanceOnly, meta = (ToolTip = "Walking Camera Shake"), Category = "Shakes")
		TSubclassOf<class UCameraShakeBase> WalkingShake;
	UPROPERTY(EditInstanceOnly, meta = (ToolTip = "Running Camera Shake"), Category = "Shakes")
		TSubclassOf<class UCameraShakeBase> RunningShake;
	UPROPERTY(EditInstanceOnly, meta = (ToolTip = "Jumping Camera Shake"), Category = "Shakes")
		TSubclassOf<class UCameraShakeBase> JumpingShake;
};

USTRUCT()
struct FFirstPersonMovementSettings {
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, meta = (ClampMin = 1.0f, ClampMax = 10000.0f, ToolTip = "Normal Movement Speed"), Category = "Movement")
		float WalkSpeed = 300.0f;
	UPROPERTY(EditInstanceOnly, meta = (ClampMin = 1.0f, ClampMax = 10000.0f, ToolTip = "Crouching Movement Speed"), Category = "Movement")
		float CrouchSpeed = 150.0f;
	UPROPERTY(EditInstanceOnly, meta = (ClampMin = 1.0f, ClampMax = 10000.0f, ToolTip = "Running Movement Speed"), Category = "Movement")
		float RunSpeed = 500.0f;
	UPROPERTY(EditInstanceOnly, meta = (ClampMin = 1.0f, ClampMax = 10000.0f, ToolTip = "Jump Velocity"), Category = "Movement")
		float JumpVelocity = 300.0f;
	UPROPERTY(EditInstanceOnly, meta = (ClampMin = 1.0f, ClampMax = 10000.0f, ToolTip = "Crouch Transition Speed"), Category = "Movement")
		float StandToCrouchTransitionSpeed = 10.0f;
	UPROPERTY(EditInstanceOnly, meta = (ClampMin = 1.0f, ClampMax = 10000.0f, ToolTip = "Crouch Toggle OFF/ON"), Category = "Movement")
		bool bToggleToCrouch = false;
};

USTRUCT()
struct FFirstPersonCameraSettings {
	GENERATED_BODY()

	UPROPERTY(EditInstanceOnly, DisplayName = "Yaw Sensitivity", meta = (ClampMin = 0.0f, UIMax = 100.0f, ToolTip = "Lower Values = Slower Speed"), Category = "Camera")
		float SensitivityX = 50.0f;
	UPROPERTY(EditInstanceOnly, DisplayName = "Pitch Sensitivity", meta = (ClampMin = 0.0f, UIMax = 100.0f, ToolTip = "Lower Values = Slower Speed"), Category = "Camera")
		float SensitivityY = 50.0f;
	UPROPERTY(EditInstanceOnly, meta = (ClampMin = 0.0f, UIMax = 100.0f, ToolTip = "Lower Values = Slower Speed"), Category = "Camera")
		float MinPitch = -90.0f;
	UPROPERTY(EditInstanceOnly, meta = (ClampMin = 0.0f, UIMax = 100.0f, ToolTip = "Lower Values = Slower Speed"), Category = "Camera")
		float MaxPitch = 90.0f;
};

UCLASS()
class FPCHARACTER_API AFPController : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPController();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	void Jump() override;
	void Landed(const FHitResult& Hit) override;
	void PossessedBy(AController* NewController) override;
	void StartCrouch();
	void StopCrouching();
	void SetupInputBindings();
	void ResetInputBindings();
	void ResetToDefaultInputBindings();

	void AddControllerYawInput(float Value) override;
	void AddControllerPitchInput(float Value) override;

	void MoveForward(float AxisValue);
	void MoveRight(float AxisValue);

	virtual void Quit();

	void UpdateCrouch(float DeltaTime);
	bool IsBlockedInCrouchStance();
	void UpdateCameraShake();

	UFUNCTION()
		virtual void Interact();
	UFUNCTION()
		void Run();
	UFUNCTION()
		void StopRunning();

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		class USpringArmComponent* SpringArmComponent;

	UPROPERTY(VisibleDefaultsOnly, Category = "Components")
		class UCameraComponent* CameraComponent;

	UPROPERTY(EditInstanceOnly, meta = (ToolTip = "Enable for Custom Key Bindings"), Category = "First Person Settings")
		bool bUseCustomKeyMappings = false;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Adjust Camera Settings"), Category = "First Person Settings")
		FFirstPersonCameraSettings Camera;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Adjust Movement Settings"), Category = "First Person Settings")
		FFirstPersonMovementSettings Movement;

	UPROPERTY(EditAnywhere, meta = (ToolTip = "Add Custom Camera Shakes"), Category = "First Person Settings")
		FCameraShakes CameraShakes;

	class UInputSettings* Input{};

private:	

	APlayerController* PlayerController;

	float OriginalCapsuleHalfHeight{};
	FVector OriginalCameraLocation;

	bool bCanUnCrouch{};
	bool bIsCrouching{};
	bool bIsRunning{};

	TArray<FInputActionKeyMapping> ActionMappings;
	TArray<FInputAxisKeyMapping> AxisMappings;
};
