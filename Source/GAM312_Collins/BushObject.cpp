// Copyright Epic Games, Inc. All Rights Reserved.

#include "BushObject.h"
#include "SurvivalCharacter.h"

ABushObject::ABushObject()
{
	// Bushes are small — fewer berries than wood on a tree
	MaxResourceAmount       = 3;
	ResourceAmount          = MaxResourceAmount;
	ResourcesPerInteraction = 1;

	// Picking berries is light work, so the stamina cost is low
	StaminaCost = 5.f;
}

void ABushObject::Interact(ASurvivalCharacter* Player)
{
	// Exit early if no berries remain or the player reference is invalid
	if (!Player || !HasResources())
	{
		return;
	}

	// Award berries to the player's inventory before decrementing the bush
	Player->AddBerry(ResourcesPerInteraction);

	// Let the base class deduct stamina, decrement ResourceAmount, and deplete if empty
	Super::Interact(Player);
}
