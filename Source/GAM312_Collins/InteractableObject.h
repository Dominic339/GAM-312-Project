// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "InteractableObject.generated.h"

// Forward declare the player character to avoid circular includes
class ASurvivalCharacter;

/**
 * Abstract base class for all collectible resource objects (Tree, Rock, Bush).
 *
 * How to use:
 *   1. Create a Blueprint child class (e.g. BP_Tree) that inherits from a
 *      concrete subclass (ATreeObject, ARockObject, ABushObject).
 *   2. Assign a Static Mesh to the MeshComponent in that Blueprint.
 *   3. Make sure the mesh collision preset blocks the Visibility channel so
 *      the player's line trace can hit it.
 *   4. Place the Blueprint in your level.
 */
UCLASS(Abstract)
class GAM312_COLLINS_API AInteractableObject : public AActor
{
	GENERATED_BODY()

public:
	AInteractableObject();

	/**
	 * Called by the player character when this object is within interaction range
	 * and the player presses the interact key.
	 *
	 * Base implementation: checks stamina, deducts it, decrements ResourceAmount,
	 * and calls OnDepleted() when the object is empty.
	 * Subclasses call Super::Interact() then award their specific resource.
	 *
	 * @param Player  The character collecting the resource (never nullptr here)
	 */
	virtual void Interact(ASurvivalCharacter* Player);

	/** Returns true while this object still has resources to give */
	bool HasResources() const { return ResourceAmount > 0; }

protected:
	// ── Mesh ─────────────────────────────────────────────────────────────────

	// Assign a mesh in the Blueprint child; collision on this mesh drives trace detection
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;

	// ── Resource Settings ─────────────────────────────────────────────────────

	// Units of resource remaining; decreases with each successful interaction
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Resource")
	int32 ResourceAmount;

	// Starting resource count; set this in the Blueprint Details panel to tune scarcity
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	int32 MaxResourceAmount;

	// How much stamina the player spends per interaction — prevents spam-collecting
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	float StaminaCost;

	// How many resource units are awarded to the player per interaction
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Resource")
	int32 ResourcesPerInteraction;

	/**
	 * Called automatically when ResourceAmount reaches zero.
	 * Hides the actor and disables its collision so the trace no longer hits it,
	 * clearly signalling to the player that the object is depleted.
	 */
	void OnDepleted();
};
