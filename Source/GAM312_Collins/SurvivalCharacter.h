// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "SurvivalCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;

/**
 * First-person player character for the survival game.
 * Handles WASD movement and mouse-look via UE5's Enhanced Input System.
 * Derive a Blueprint from this class and assign the MoveAction, LookAction,
 * and DefaultMappingContext assets created in the editor.
 */
UCLASS()
class GAM312_COLLINS_API ASurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASurvivalCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	// Camera positioned at eye level; bUsePawnControlRotation lets mouse input rotate it
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

	// Assign the IMC_Default asset here in the derived Blueprint
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	// Axis2D input action bound to WASD keys (X = strafe, Y = forward)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	// Axis2D input action bound to mouse movement (X = yaw, Y = pitch)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	// Top walking speed in cm/s; tunable in the Blueprint Details panel
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed;

	// Called each frame a movement key is held — translates 2D input into world-space movement
	void Move(const FInputActionValue& Value);

	// Called each frame the mouse moves — rotates the controller (and thus the camera)
	void Look(const FInputActionValue& Value);
};
