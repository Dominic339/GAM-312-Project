// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildingTypes.h"
#include "BuildMenuWidget.generated.h"

class UButton;
class UTextBlock;
class ASurvivalCharacter;

/**
 * Build menu HUD. Lists Wall/Floor/Ceiling with their resource costs and current
 * inventory, and relays button clicks back to the owning character.
 *
 * All click-handling and text updates are done here in C++ — the WBP_BuildMenu
 * asset only needs a Button and TextBlock added in the Designer for each
 * property below, named to match exactly (BindWidget matches by variable name).
 * No Blueprint graph logic is required.
 *
 * Setup in the editor:
 *   1. Create Widget Blueprint WBP_BuildMenu with parent class BuildMenuWidget.
 *   2. Add widgets named: BuildWallButton, BuildFloorButton, BuildCeilingButton
 *      (Button), and WoodCountText, StoneCountText, WallCostText, FloorCostText,
 *      CeilingCostText (Text Block).
 *   3. Assign WBP_BuildMenu to BP_SurvivalCharacter's BuildMenuWidgetClass property.
 */
UCLASS()
class GAM312_COLLINS_API UBuildMenuWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	// Set by ASurvivalCharacter immediately after CreateWidget
	UPROPERTY(BlueprintReadOnly, Category = "Building")
	ASurvivalCharacter* OwningCharacter;

	/** Refreshes all resource/cost text from OwningCharacter. Call after any inventory change. */
	void UpdateInventoryDisplay();

protected:
	virtual void NativeConstruct() override;

	// ── Bind widgets with these exact names in WBP_BuildMenu's Designer ──────
	UPROPERTY(meta = (BindWidget))
	UButton* BuildWallButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BuildFloorButton;

	UPROPERTY(meta = (BindWidget))
	UButton* BuildCeilingButton;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WoodCountText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* StoneCountText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* WallCostText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* FloorCostText;

	UPROPERTY(meta = (BindWidget))
	UTextBlock* CeilingCostText;

private:
	UFUNCTION()
	void OnBuildWallClicked();

	UFUNCTION()
	void OnBuildFloorClicked();

	UFUNCTION()
	void OnBuildCeilingClicked();
};
