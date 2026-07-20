// Copyright Epic Games, Inc. All Rights Reserved.

#include "TreeObject.h"
#include "SurvivalCharacter.h"

ATreeObject::ATreeObject()
{
	// Trees carry more wood than other resources to reflect their size
	MaxResourceAmount       = 5;
	ResourceAmount          = MaxResourceAmount;
	ResourcesPerInteraction = 1;

	// Chopping a tree costs more stamina than picking berries
	StaminaCost = 15.f;
}

void ATreeObject::Interact(ASurvivalCharacter* Player)
{
	// Exit early if there is no wood left or the player is invalid
	if (!Player || !HasResources())
	{
		return;
	}

	// Award wood to the player's inventory before decrementing the tree
	Player->AddWood(ResourcesPerInteraction);

	// Let the base class deduct stamina, decrement ResourceAmount, and deplete if empty
	Super::Interact(Player);
}
