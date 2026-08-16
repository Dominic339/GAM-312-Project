// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ObjectiveWidget.generated.h"

class UProgressBar;
class UTextBlock;
class ASurvivalCharacter;

/**
 * Always-on HUD tracking the player's two win-condition objectives:
 * materials collected and shelter parts built. ASurvivalCharacter calls
 * UpdateObjectives() whenever either count changes.
 *
 * All logic lives here in C++ — the WBP_ObjectiveHUD asset only needs the
 * widgets below added in the Designer, named to match exactly (BindWidget
 * matches by variable name). No graph logic required.
 *
 * Setup in the editor:
 *   1. Create Widget Blueprint WBP_ObjectiveHUD with parent class ObjectiveWidget.
 *   2. Add: MaterialsBar, PartsBar (Progress Bar); MaterialsText, PartsText,
 *      CompletionText (Text Block).
 *   3. Assign WBP_ObjectiveHUD to BP_SurvivalCharacter's ObjectiveWidgetClass property.
 */
UCLASS()
class GAM312_COLLINS_API UObjectiveWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set by ASurvivalCharacter immediately after CreateWidget
	UPROPERTY(BlueprintReadOnly, Category = "Objectives")
	ASurvivalCharacter* OwningCharacter;

	/** Refreshes progress bars/text from OwningCharacter and shows CompletionText once both objectives are met */
	void UpdateObjectives();

protected:
	virtual void NativeConstruct() override;

	// ── Bind widgets with these exact names in WBP_ObjectiveHUD's Designer ───
	UPROPERTY(meta = (BindWidget))
	UProgressBar* MaterialsBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* MaterialsText;

	UPROPERTY(meta = (BindWidget))
	UProgressBar* PartsBar;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* PartsText;

	// Hidden until both objectives are complete
	UPROPERTY(meta = (BindWidget))
	UTextBlock* CompletionText;
};
