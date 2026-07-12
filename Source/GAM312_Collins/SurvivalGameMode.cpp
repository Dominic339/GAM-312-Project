// Copyright Epic Games, Inc. All Rights Reserved.

#include "SurvivalGameMode.h"
#include "SurvivalCharacter.h"

ASurvivalGameMode::ASurvivalGameMode()
{
	// Tell UE which pawn class to spawn when a player joins or the level starts
	DefaultPawnClass = ASurvivalCharacter::StaticClass();
}
