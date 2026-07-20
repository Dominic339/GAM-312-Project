// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "RockObject.generated.h"

/**
 * A collectible rock formation that yields Stone each time the player interacts.
 *
 * Setup in the editor:
 *   1. Create BP_Rock as a Blueprint child of ARockObject.
 *   2. Assign a rock Static Mesh to the MeshComponent.
 *   3. Place BP_Rock actors around the level.
 *
 * The rock disappears when all stone has been mined (handled by base class).
 */
UCLASS()
class GAM312_COLLINS_API ARockObject : public AInteractableObject
{
	GENERATED_BODY()

public:
	ARockObject();

	/**
	 * Awards Stone to the player then delegates depletion bookkeeping to the base class.
	 * @param Player  The character collecting stone
	 */
	virtual void Interact(ASurvivalCharacter* Player) override;
};
