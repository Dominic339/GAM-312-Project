// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "InteractableObject.h"
#include "TreeObject.generated.h"

/**
 * A collectible tree that yields Wood each time the player interacts with it.
 *
 * Setup in the editor:
 *   1. Create BP_Tree as a Blueprint child of ATreeObject.
 *   2. Assign a tree Static Mesh to the MeshComponent.
 *   3. Place BP_Tree actors around the level.
 *
 * The tree disappears when all wood has been harvested (handled by base class).
 */
UCLASS()
class GAM312_COLLINS_API ATreeObject : public AInteractableObject
{
	GENERATED_BODY()

public:
	ATreeObject();

	/**
	 * Awards Wood to the player then delegates depletion bookkeeping to the base class.
	 * @param Player  The character collecting wood
	 */
	virtual void Interact(ASurvivalCharacter* Player) override;
};
