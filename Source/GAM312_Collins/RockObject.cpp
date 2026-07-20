// Copyright Epic Games, Inc. All Rights Reserved.

#include "RockObject.h"
#include "SurvivalCharacter.h"

ARockObject::ARockObject()
{
	// Rocks yield a moderate amount of stone
	MaxResourceAmount       = 4;
	ResourceAmount          = MaxResourceAmount;
	ResourcesPerInteraction = 1;

	// Mining rock is physically demanding
	StaminaCost = 20.f;
}

void ARockObject::Interact(ASurvivalCharacter* Player)
{
	// Exit early if no stone remains or the player reference is invalid
	if (!Player || !HasResources())
	{
		return;
	}

	// Award stone to the player's inventory before decrementing the rock
	Player->AddStone(ResourcesPerInteraction);

	// Let the base class deduct stamina, decrement ResourceAmount, and deplete if empty
	Super::Interact(Player);
}
