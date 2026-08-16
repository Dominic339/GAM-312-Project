// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BuildingTypes.h"
#include "BuildableObject.generated.h"

/**
 * A single placeable shelter piece (wall, floor, or ceiling).
 *
 * Setup in the editor:
 *   1. Create BP_Wall, BP_Floor, and BP_Ceiling as Blueprint children of this class.
 *   2. In each one, assign a Static Mesh to MeshComponent and set BuildType
 *      to the matching enum value (Wall / Floor / Ceiling) in the Details panel.
 *   3. Assign BP_Wall/BP_Floor/BP_Ceiling to the matching TSubclassOf properties
 *      on BP_SurvivalCharacter (WallClass / FloorClass / CeilingClass).
 *
 * ASurvivalCharacter spawns one of these in "preview mode" (no collision) while
 * the player is aiming placement, then switches it to full collision once placed.
 */
UCLASS()
class GAM312_COLLINS_API ABuildableObject : public AActor
{
	GENERATED_BODY()

public:
	ABuildableObject();

	// Which shelter piece this Blueprint represents; set per Blueprint child
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building")
	EBuildPieceType BuildType;

	/**
	 * Toggles between ghost-preview (no collision, so it can't block the placement
	 * trace or the player) and a finalized, fully-collidable placed piece.
	 */
	void SetPreviewMode(bool bIsPreview);

protected:
	// Assign a mesh in the Blueprint child; simple collision on this mesh blocks the player once placed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	UStaticMeshComponent* MeshComponent;
};
