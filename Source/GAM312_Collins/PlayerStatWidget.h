// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "PlayerStatWidget.generated.h"

class UProgressBar;
class ASurvivalCharacter;

/**
 * Always-on player HUD showing Health, Hunger, and Stamina as progress bars.
 * ASurvivalCharacter calls UpdateStats() every tick so the bars track the
 * stats in real time as they decay/regenerate.
 *
 * All logic lives here in C++ — the WBP_PlayerStatHUD asset only needs a
 * Progress Bar added in the Designer for each property below, named to
 * match exactly (BindWidget matches by variable name). No graph logic
 * required.
 *
 * Setup in the editor:
 *   1. Create Widget Blueprint WBP_PlayerStatHUD with parent class PlayerStatWidget.
 *   2. Add three Progress Bar widgets named: HealthBar, HungerBar, StaminaBar.
 *   3. Assign WBP_PlayerStatHUD to BP_SurvivalCharacter's PlayerStatWidgetClass property.
 */
UCLASS()
class GAM312_COLLINS_API UPlayerStatWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set by ASurvivalCharacter immediately after CreateWidget
	UPROPERTY(BlueprintReadOnly, Category = "Stats")
	ASurvivalCharacter* OwningCharacter;

	/** Reads Health/Hunger/Stamina percentages from OwningCharacter and pushes them to the progress bars */
	void UpdateStats();

protected:
	// ── Bind widgets with these exact names in WBP_PlayerStatHUD's Designer ──
	UPROPERTY(meta = (BindWidget))
	UProgressBar* HealthBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* HungerBar;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* StaminaBar;
};
