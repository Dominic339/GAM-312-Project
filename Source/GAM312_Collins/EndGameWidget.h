// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EndGameWidget.generated.h"

class UButton;

/**
 * Shared base for the Win and Lose end-game screens. Handles Restart/Quit
 * button clicks entirely in C++ — WBP_Win and WBP_Lose need no graph logic,
 * just a Button named RestartButton and one named QuitButton, plus whatever
 * static "You Win!" / "You Lose!" text and background art distinguishes them.
 *
 * Setup in the editor:
 *   1. Create WBP_Win and WBP_Lose, both with parent class EndGameWidget.
 *   2. Add a Button named RestartButton and one named QuitButton to each.
 *   3. Add a Text Block to each with literal text "You Win!" / "You Lose!" —
 *      purely cosmetic, no C++ binding needed since it never changes at runtime.
 *   4. Assign WBP_Win to BP_SurvivalCharacter's WinWidgetClass, and WBP_Lose
 *      to LoseWidgetClass.
 */
UCLASS(Abstract)
class GAM312_COLLINS_API UEndGameWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;

	// ── Bind widgets with these exact names in the Designer ──────────────────
	UPROPERTY(meta = (BindWidget))
	UButton* RestartButton;

	UPROPERTY(meta = (BindWidget))
	UButton* QuitButton;

private:
	UFUNCTION()
	void OnRestartClicked();

	UFUNCTION()
	void OnQuitClicked();
};
