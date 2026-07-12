// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "SurvivalGameMode.generated.h"

/**
 * Default game mode for the survival game.
 * Sets SurvivalCharacter as the pawn spawned for every player at game start.
 */
UCLASS()
class GAM312_COLLINS_API ASurvivalGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ASurvivalGameMode();
};
