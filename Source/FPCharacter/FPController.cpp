// Fill out your copyright notice in the Description page of Project Settings.


#include "FPController.h"

// Sets default values
AFPController::AFPController()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AFPController::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AFPController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AFPController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

