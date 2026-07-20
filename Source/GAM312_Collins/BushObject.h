// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "BushObject.generated.h"

/**
 * A collectible bush that yields Berries each time the player interacts.
 *
 * Setup in the editor:
 *   1. Create BP_Bush as a Blueprint child of ABushObject.
 *   2. Assign a bush Static Mesh to the MeshComponent.
 *   3. Place BP_Bush actors around the level.
 *
 * The bush disappears when all berries have been picked (handled by base class).
 */
UCLASS()
class GAM312_COLLINS_API ABushObject : public AInteractableObject
{
	GENERATED_BODY()

public:
	ABushObject();

	/**
	 * Awards Berries to the player then delegates depletion bookkeeping to the base class.
	 * @param Player  The character collecting berries
	 */
	virtual void Interact(ASurvivalCharacter* Player) override;
};
