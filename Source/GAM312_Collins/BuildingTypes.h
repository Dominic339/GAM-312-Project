// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "BuildingTypes.generated.h"

/** Identifies which shelter piece is selected/placed by the building system. */
UENUM(BlueprintType)
enum class EBuildPieceType : uint8
{
	None    UMETA(DisplayName = "None"),
	Wall    UMETA(DisplayName = "Wall"),
	Floor   UMETA(DisplayName = "Floor"),
	Ceiling UMETA(DisplayName = "Ceiling")
};

/** Resource amounts required to craft one shelter piece. */
USTRUCT(BlueprintType)
struct FBuildingCost
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
	int32 WoodCost = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Cost")
	int32 StoneCost = 0;
};
