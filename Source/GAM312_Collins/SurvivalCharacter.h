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
 *
 * Responsibilities:
 *   - WASD movement and mouse-look via UE5 Enhanced Input
 *   - Stat management: Health, Hunger, Stamina tick each frame
 *   - Resource inventory: Wood, Stone, Berry incremented on collection
 *   - Line-trace interaction: pressing E fires a trace and calls Interact()
 *     on any AInteractableObject within range
 *
 * Setup in the editor:
 *   1. Derive BP_SurvivalCharacter from this class.
 *   2. Assign IMC_Default, IA_Move, IA_Look, IA_Interact in the Details panel.
 *   3. Set BP_SurvivalCharacter as the Default Pawn Class in the Game Mode.
 */
UCLASS()
class GAM312_COLLINS_API ASurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASurvivalCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	// ── Camera ───────────────────────────────────────────────────────────────

	// Positioned at eye height; bUsePawnControlRotation makes mouse input aim it
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

	// ── Input ─────────────────────────────────────────────────────────────────

	// Assign IMC_Default in the Blueprint Details panel
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	// Axis2D action — X = strafe, Y = forward/back (bound to WASD)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	// Axis2D action — X = yaw, Y = pitch (bound to mouse)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	// Digital (bool) action — bound to E key; triggers the interaction trace
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	// ── Player Stats ──────────────────────────────────────────────────────────

	// Current health — drains when hunger reaches zero; game over at 0
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float Health;

	// Upper bound for health; default 100
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	// Current hunger — decreases passively over time
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float Hunger;

	// Upper bound for hunger; default 100
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float MaxHunger;

	// Current stamina — spent on resource collection, recovers passively
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float Stamina;

	// Upper bound for stamina; default 100
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float MaxStamina;

	// How many hunger units per second are lost (tune in Blueprint Details)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float HungerDecayRate;

	// How many health units per second are lost when Hunger == 0
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float HealthDecayRate;

	// How many stamina units per second are restored when not collecting
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float StaminaRestoreRate;

	// ── Resource Inventory ────────────────────────────────────────────────────

	// Units of wood collected from trees
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 Wood;

	// Units of stone collected from rocks
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 Stone;

	// Units of berry collected from bushes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 Berry;

	// ── Interaction ───────────────────────────────────────────────────────────

	// Maximum distance (cm) at which the player can interact with objects
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	float InteractRange;

	// ── Movement ──────────────────────────────────────────────────────────────

	// Top walking speed in cm/s; tunable in Blueprint Details
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed;

	// ── Private helpers ───────────────────────────────────────────────────────

	// Translates 2D move input into world-space movement each tick
	void Move(const FInputActionValue& Value);

	// Translates 2D look input into controller rotation each tick
	void Look(const FInputActionValue& Value);

	// Fires a line trace from the camera; calls Interact() on any hit interactable
	void TryInteract();

	// Decreases Hunger each frame; clamps to [0, MaxHunger]
	void UpdateHunger(float DeltaTime);

	// Drains Health each frame only when Hunger is zero; clamps to [0, MaxHealth]
	void UpdateHealth(float DeltaTime);

	// Passively restores Stamina each frame; clamps to [0, MaxStamina]
	void UpdateStamina(float DeltaTime);

public:
	// ── Public Interface (used by AInteractableObject subclasses) ─────────────

	/** Read-only access to current stamina so interactables can gate collection */
	float GetStamina() const { return Stamina; }

	/** Deduct Amount from Stamina; result is clamped to [0, MaxStamina] */
	void UseStamina(float Amount);

	/** Add Amount units of wood to the inventory */
	void AddWood(int32 Amount);

	/** Add Amount units of stone to the inventory */
	void AddStone(int32 Amount);

	/** Add Amount units of berry to the inventory */
	void AddBerry(int32 Amount);
};
