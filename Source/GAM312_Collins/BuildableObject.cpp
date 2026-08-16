// Copyright Epic Games, Inc. All Rights Reserved.

#include "BuildableObject.h"
#include "Components/StaticMeshComponent.h"

ABuildableObject::ABuildableObject()
{
	// Placed pieces are static once finalized — no per-frame logic needed
	PrimaryActorTick.bCanEverTick = false;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	RootComponent = MeshComponent;

	BuildType = EBuildPieceType::None;

	// Default to full (simple) collision; SetPreviewMode(true) disables this while ghosting
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	MeshComponent->SetCollisionResponseToAllChannels(ECR_Block);
}

void ABuildableObject::SetPreviewMode(bool bIsPreview)
{
	if (bIsPreview)
	{
		// Ghost preview: disable collision so it doesn't block its own placement trace or the player
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	}
	else
	{
		// Finalized piece: full collision so the player and future traces block against it
		MeshComponent->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	}
}
