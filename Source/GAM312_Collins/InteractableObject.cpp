// Copyright Epic Games, Inc. All Rights Reserved.

#include "InteractableObject.h"
#include "SurvivalCharacter.h"
#include "Components/StaticMeshComponent.h"

AInteractableObject::AInteractableObject()
{
	// This actor does not need to tick — all logic is event-driven (player interaction)
	PrimaryActorTick.bCanEverTick = false;

	// Create the mesh component and make it the root so the actor has a world transform
	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	// Sensible defaults — subclasses and Blueprints can override these in Details
	MaxResourceAmount       = 5;
	ResourceAmount          = MaxResourceAmount;
	StaminaCost             = 10.f;
	ResourcesPerInteraction = 1;
}

void AInteractableObject::Interact(ASurvivalCharacter* Player)
{
	// Safety guard: do nothing if the player reference is invalid or object is empty
	if (!Player || ResourceAmount <= 0)
	{
		return;
	}

	// Reject the interaction if the player lacks enough stamina to collect
	if (Player->GetStamina() < StaminaCost)
	{
		// Player is too tired — return without consuming resources or stamina
		return;
	}

	// Spend the player's stamina for performing this collection action
	Player->UseStamina(StaminaCost);

	// Remove the harvested amount from what remains in this object
	ResourceAmount -= ResourcesPerInteraction;

	// Clamp to zero so we never store a negative value
	if (ResourceAmount < 0)
	{
		ResourceAmount = 0;
	}

	// If the object is now empty, hide it and disable collision
	if (ResourceAmount <= 0)
	{
		OnDepleted();
	}
}

void AInteractableObject::OnDepleted()
{
	// Hide the mesh so the player can see the object is exhausted
	SetActorHiddenInGame(true);

	// Disable all collision so the player's trace no longer hits this actor
	SetActorEnableCollision(false);
}
