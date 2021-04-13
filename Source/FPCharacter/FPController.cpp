// Fill out your copyright notice in the Description page of Project Settings.


#include "FPController.h"

#include "Components/InputComponent.h"
#include "Components/CapsuleComponent.h"

#include "Camera/CameraComponent.h"

#include "GameFramework/Controller.h"
#include "GameFramework/GameUserSettings.h"
#include "GameFramework/InputSettings.h"
#include "GameFramework/SpringArmComponent.h"

#include "Kismet/GameplayStatics.h"

// Sets default values
AFPController::AFPController()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(FName("SpringArmComponent"));
	SpringArmComponent->TargetArmLength = 0.0f;
	SpringArmComponent->SetupAttachment(GetCapsuleComponent());

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(FName("CameraComponent"));
	CameraComponent->SetRelativeLocation(FVector(0.0f, 0.0f, 70.0f));
	CameraComponent->bUsePawnControlRotation = true;
	CameraComponent->SetupAttachment(SpringArmComponent);

	GetCharacterMovement()->MaxWalkSpeed = 300.0f;
	GetCharacterMovement()->JumpZVelocity = 300.0f;
	GetCharacterMovement()->AirControl = 0.1f;

	AutoPossessPlayer = EAutoReceiveInput::Player0;
	AutoReceiveInput = EAutoReceiveInput::Player0;

	bCanUnCrouch = true;
}

// Called when the game starts or when spawned
void AFPController::BeginPlay()
{
	Super::BeginPlay();
	
	//Get Access to Input Settings
	Input = const_cast<UInputSettings*>(GetDefault<UInputSettings>());

	GetCharacterMovement()->MaxWalkSpeed = Movement.WalkSpeed;
	GetCharacterMovement()->JumpZVelocity = Movement.JumpVelocity;

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (CameraManager) {
		CameraManager->ViewPitchMin = Camera.MinPitch;
		CameraManager->ViewPitchMax = Camera.MaxPitch;
	}

	// Initialization
	OriginalCameraLocation = CameraComponent->GetRelativeLocation();
	OriginalCapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	bCanUnCrouch = true;

	// Input Setup
	SetupInputBindings();
}

// Called every frame
void AFPController::Tick(const float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateCameraShake();
	UpdateCrouch(DeltaTime);
}

// Called to bind functionality to input
void AFPController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Axis Bindings
	PlayerInputComponent->BindAxis(FName("MoveForward"), this, &AFPController::MoveForward);
	PlayerInputComponent->BindAxis(FName("MoveRight"), this, &AFPController::MoveRight);
	PlayerInputComponent->BindAxis(FName("Turn"), this, &AFPController::AddControllerYawInput);
	PlayerInputComponent->BindAxis(FName("LookUp"), this, &AFPController::AddControllerPitchInput);

	// Action Bindings
	PlayerInputComponent->BindAction(FName("Jump"), IE_Pressed, this, &AFPController::Jump);
	PlayerInputComponent->BindAction(FName("Jump"), IE_Released, this, &AFPController::StopJumping);
	PlayerInputComponent->BindAction(FName("Run"), IE_Pressed, this, &AFPController::Run);
	PlayerInputComponent->BindAction(FName("Run"), IE_Released, this, &AFPController::StopRunning);
	PlayerInputComponent->BindAction(FName("Crouch"), IE_Pressed, this, &AFPController::StartCrouch);
	PlayerInputComponent->BindAction(FName("Crouch"), IE_Released, this, &AFPController::StopCrouching);
	PlayerInputComponent->BindAction(FName("Interact"), IE_Pressed, this, &AFPController::Interact);
	PlayerInputComponent->BindAction(FName("Quit"), IE_Pressed, this, &AFPController::Quit);
}

void AFPController::Jump() {
	if (!bIsCrouching) {
		Super::Jump();
		PlayerController->ClientStartCameraShake(CameraShakes.JumpingShake);
	}
}

void AFPController::Landed(const FHitResult& Hit) {
	if (!bIsCrouching) {
		Super::Landed(Hit);
		PlayerController->ClientStartCameraShake(CameraShakes.JumpingShake, 3.0f);
	}
}

void AFPController::PossessedBy(AController* NewController) {
	Super::PossessedBy(NewController);
	PlayerController = Cast<APlayerController>(NewController);
}

void AFPController::StartCrouch() {
	if (GetCharacterMovement()->IsMovingOnGround() && bCanUnCrouch) {
		bIsCrouching = !bIsCrouching;
		GetCharacterMovement()->MaxWalkSpeed = bIsCrouching ? Movement.CrouchSpeed : Movement.WalkSpeed;
	}
}

void AFPController::StopCrouching() {
	if (!Movement.bToggleToCrouch && bCanUnCrouch) {
		GetCharacterMovement()->MaxWalkSpeed = Movement.WalkSpeed;
		bIsCrouching = false;
	}
}

void AFPController::MoveForward(const float AxisValue) {
	if (Controller) {
		FRotator ForwardRotation = Controller->GetControlRotation();
		if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
			ForwardRotation.Pitch = 0.0f;
		const FVector Direction = FRotationMatrix(ForwardRotation).GetScaledAxis(EAxis::X);
		AddMovementInput(Direction, AxisValue);
	}
}

void AFPController::MoveRight(const float AxisValue) {
	if (Controller) {
		const FRotator RightRotation = Controller->GetControlRotation();
		const FVector Direction = FRotationMatrix(RightRotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(Direction, AxisValue);
	}
}

void AFPController::Run() {
	if (!bIsCrouching) {
		GetCharacterMovement()->MaxWalkSpeed = Movement.RunSpeed;
		bIsRunning = true;
	}
}

void AFPController::StopRunning() {
	if (!bIsCrouching) {
		GetCharacterMovement()->MaxWalkSpeed = Movement.WalkSpeed;
		bIsRunning = false;
	}
}

void AFPController::UpdateCrouch(const float DeltaTime) {
	if (bIsCrouching) {
		const FVector NewLocation = FMath::Lerp(CameraComponent->GetRelativeLocation(), FVector(0.0f, 0.0f, 30.0f), Movement.StandToCrouchTransitionSpeed * DeltaTime);
		const float NewHalfHeight = FMath::Lerp(GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), OriginalCapsuleHalfHeight / 2.0f, Movement.StandToCrouchTransitionSpeed * DeltaTime);

		CameraComponent->SetRelativeLocation(NewLocation);
		GetCapsuleComponent()->SetCapsuleHalfHeight(NewHalfHeight);

		if (IsBlockedInCrouchStance())
			bCanUnCrouch = false;
		else
			bCanUnCrouch = true;
	}
	else {
		const FVector NewLocation = FMath::Lerp(CameraComponent->GetRelativeLocation(), OriginalCameraLocation, Movement.StandToCrouchTransitionSpeed * DeltaTime);
		const float NewHalfHeight = FMath::Lerp(GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), OriginalCapsuleHalfHeight, Movement.StandToCrouchTransitionSpeed * DeltaTime);

		CameraComponent->SetRelativeLocation(NewLocation);
		GetCapsuleComponent()->SetCapsuleHalfHeight(NewHalfHeight);
	}
}

bool AFPController::IsBlockedInCrouchStance() {
	FHitResult HitResult;
	const FVector RayLength = FVector(0.0f, 0.0f, 130.0f);
	return GetWorld()->LineTraceSingleByChannel(HitResult, GetActorLocation(), GetActorLocation() + RayLength, ECC_Visibility);
}

void AFPController::UpdateCameraShake() {
	if (PlayerController) {
		if (GetVelocity().Size() > 0 && CanJump())
			PlayerController->ClientStartCameraShake(CameraShakes.WalkingShake, 2.0f);
		else
			PlayerController->ClientStartCameraShake(CameraShakes.IdleShake, 1.0f);

		if (GetVelocity().Size() > 0 && GetCharacterMovement()->MaxWalkSpeed >= Movement.RunSpeed && CanJump())
			PlayerController->ClientStartCameraShake(CameraShakes.RunningShake, 1.0f);
	}
}

void AFPController::Quit() {
	UKismetSystemLibrary::QuitGame(GetWorld(), Cast<APlayerController>(GetController()), EQuitPreference::Quit, true);
}

void AFPController::Interact() {
	if (GEngine)
		GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, TEXT("Interaction Not Yet Implemented"));
}

void AFPController::SetupInputBindings() {
	ActionMappings = Input->GetActionMappings();
	AxisMappings = Input->GetAxisMappings();

	if (ActionMappings.Num() > 0 || AxisMappings.Num() > 0) {
		if (!bUseCustomKeyMappings)
			ResetToDefaultInputBindings();
		return;
	}
	ResetToDefaultInputBindings();
}

void AFPController::ResetInputBindings() {
	for (const auto& Action : ActionMappings)
		Input->RemoveActionMapping(Action);

	for (const auto& Axis : AxisMappings)
		Input->RemoveAxisMapping(Axis);
}

void AFPController::ResetToDefaultInputBindings()
{
	// Clear all the action and axis mappings
	ResetInputBindings();

	// Populate the map with action and axis mappings
	TArray<FName> ActionNames;
	TArray<FName> AxisNames;
	TArray<FKey> ActionKeys;
	TArray<FKey> AxisKeys;
	TMap<FKey, FName> ActionMap;
	TMap<FKey, FName> AxisMap;
	ActionMap.Add(EKeys::SpaceBar, FName("Jump"));
	ActionMap.Add(EKeys::E, FName("Interact"));
	ActionMap.Add(EKeys::Escape, FName("Escape"));
	ActionMap.Add(EKeys::LeftShift, FName("Run"));
	ActionMap.Add(EKeys::LeftControl, FName("Crouch"));
	ActionMap.Add(EKeys::C, FName("Crouch"));
	AxisMap.Add(EKeys::MouseX, FName("Turn"));
	AxisMap.Add(EKeys::MouseY, FName("LookUp"));
	AxisMap.Add(EKeys::W, FName("MoveForward"));
	AxisMap.Add(EKeys::S, FName("MoveForward"));
	AxisMap.Add(EKeys::A, FName("MoveRight"));
	AxisMap.Add(EKeys::D, FName("MoveRight"));

	// Lambda function to set the new action mappings
	const auto SetActionMapping = [&](const FName Name, const FKey Key)
	{
		FInputActionKeyMapping ActionMapping;
		ActionMapping.ActionName = Name;
		ActionMapping.Key = bUseCustomKeyMappings ? EKeys::NAME_KeyboardCategory : Key;
		Input->AddActionMapping(ActionMapping);
	};

	// Lambda function to set the new axis mappings
	const auto SetAxisMapping = [&](const FName Name, const FKey Key, const float Scale)
	{
		FInputAxisKeyMapping AxisMapping;
		AxisMapping.AxisName = Name;
		AxisMapping.Key = bUseCustomKeyMappings ? EKeys::NAME_KeyboardCategory : Key;
		AxisMapping.Scale = Scale;
		Input->AddAxisMapping(AxisMapping);
	};

	// Loop through the entire ActionMap and assign action inputs
	ActionMap.GenerateKeyArray(ActionKeys);
	ActionMap.GenerateValueArray(ActionNames);
	for (int32 i = 0; i < ActionMap.Num(); i++)
	{
		SetActionMapping(ActionNames[i], ActionKeys[i]);
	}

	// Loop through the entire AxisMap and assign axis inputs
	AxisMap.GenerateKeyArray(AxisKeys);
	AxisMap.GenerateValueArray(AxisNames);
	for (int32 i = 0; i < AxisMap.Num(); i++)
	{
		const bool bIsNegativeScale = AxisKeys[i] == EKeys::S || AxisKeys[i] == EKeys::A || AxisKeys[i] == EKeys::MouseY;

		SetAxisMapping(AxisNames[i], AxisKeys[i], bIsNegativeScale ? -1.0f : 1.0f);
	}

	// Save to input config file
	Input->SaveKeyMappings();

	// Update in Project Settings -> Engine -> Input
	Input->ForceRebuildKeymaps();
}

void AFPController::AddControllerYawInput(const float Value) {
	return Super::AddControllerYawInput(Value * Camera.SensitivityX * GetWorld()->GetDeltaSeconds());
}

void AFPController::AddControllerPitchInput(const float Value) {
	Super::AddControllerPitchInput(Value * Camera.SensitivityY * GetWorld()->GetDeltaSeconds());
}