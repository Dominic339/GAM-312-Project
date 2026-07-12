// Copyright Epic Games, Inc. All Rights Reserved.

#include "SurvivalCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"

ASurvivalCharacter::ASurvivalCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Attach the camera at a standard eye height (64 units ≈ 170 cm scaled)
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetRootComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));

	// Camera follows controller rotation so mouse input directly aims the view
	FirstPersonCamera->bUsePawnControlRotation = true;

	// Yaw rotates the whole character so movement always goes where we're looking
	bUseControllerRotationYaw  = true;
	// Pitch and roll only affect the camera, not the capsule
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll  = false;

	// Default walk speed; overridable per-Blueprint in the Details panel
	WalkSpeed = 600.f;
}

void ASurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Apply the walk speed to the movement component now that the world exists
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Register our mapping context so the engine knows which keys map to which actions
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// Priority 0 — the only context active; higher numbers override lower ones
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}
}

void ASurvivalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// UE5 Enhanced Input replaces the old BindAxis/BindAction API
	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Triggered fires every tick a key is held, which is what continuous movement needs
		EIC->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASurvivalCharacter::Move);
		EIC->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASurvivalCharacter::Look);
	}
}

void ASurvivalCharacter::Move(const FInputActionValue& Value)
{
	// MoveAction must be set to Value Type: Axis2D in the editor
	// X component drives left/right strafe; Y component drives forward/back
	const FVector2D MovementInput = Value.Get<FVector2D>();

	if (Controller)
	{
		AddMovementInput(GetActorForwardVector(), MovementInput.Y);
		AddMovementInput(GetActorRightVector(),   MovementInput.X);
	}
}

void ASurvivalCharacter::Look(const FInputActionValue& Value)
{
	// LookAction must be set to Value Type: Axis2D in the editor
	// X drives horizontal (yaw) rotation; Y drives vertical (pitch) rotation
	const FVector2D LookInput = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookInput.X);
		AddControllerPitchInput(LookInput.Y);
	}
}
