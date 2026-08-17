// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "InputActionValue.h"
#include "BuildingTypes.h"
#include "SurvivalCharacter.generated.h"

class UCameraComponent;
class UInputMappingContext;
class UInputAction;
class ABuildableObject;
class UBuildMenuWidget;
class UPlayerStatWidget;
class UObjectiveWidget;
class UEndGameWidget;

/**
 * First-person player character for the survival game.
 *
 * Responsibilities:
 *   - WASD movement and mouse-look via UE5 Enhanced Input
 *   - Stat management: Health, Hunger, Stamina tick each frame
 *   - Resource inventory: Wood, Stone, Berry incremented on collection
 *   - Line-trace interaction: pressing E fires a trace and calls Interact()
 *     on any AInteractableObject within range
 *   - Building system: opens a build menu widget, spawns a ghost preview of
 *     the selected shelter piece (Wall/Floor/Ceiling), and places it via a
 *     second trace once the player can afford its resource cost
 *   - Win/lose: a countdown timer runs each tick; completing both objectives
 *     before it expires shows the Win screen, while running out of time or
 *     Health reaching 0 shows the Lose screen
 *
 * Setup in the editor:
 *   1. Derive BP_SurvivalCharacter from this class.
 *   2. Assign IMC_Default, IA_Move, IA_Look, IA_Interact, IA_ToggleBuild,
 *      IA_PlaceBuildable in the Details panel.
 *   3. Set BP_SurvivalCharacter as the Default Pawn Class in the Game Mode.
 *   4. Assign WallClass/FloorClass/CeilingClass (BP_Wall/BP_Floor/BP_Ceiling)
 *      and BuildMenuWidgetClass (WBP_BuildMenu) in the Details panel.
 *   5. Assign PlayerStatWidgetClass (WBP_PlayerStatHUD) in the Details panel.
 *   6. Assign ObjectiveWidgetClass (WBP_ObjectiveHUD) in the Details panel.
 *   7. Assign WinWidgetClass (WBP_Win) and LoseWidgetClass (WBP_Lose) in the Details panel.
 */
UCLASS()
class GAM312_COLLINS_API ASurvivalCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	ASurvivalCharacter();

protected:
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(UInputComponent* PlayerInputComponent) override;

private:
	// ── Camera ───────────────────────────────────────────────────────────────

	// Positioned at eye height; bUsePawnControlRotation makes mouse input aim it
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCamera;

	// ── Input ─────────────────────────────────────────────────────────────────

	// Assign IMC_Default in the Blueprint Details panel
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputMappingContext* DefaultMappingContext;

	// Axis2D action — X = strafe, Y = forward/back (bound to WASD)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* MoveAction;

	// Axis2D action — X = yaw, Y = pitch (bound to mouse)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* LookAction;

	// Digital (bool) action — bound to E key; triggers the interaction trace
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* InteractAction;

	// Digital (bool) action — bound to a key (e.g. B); opens/closes the build menu, or cancels an active placement
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* ToggleBuildMenuAction;

	// Digital (bool) action — bound to a key/mouse button; confirms placement of the currently previewed piece
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	UInputAction* PlaceBuildableAction;

	// ── Player Stats ──────────────────────────────────────────────────────────

	// Current health — drains when hunger reaches zero; game over at 0
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float Health;

	// Upper bound for health; default 100
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float MaxHealth;

	// Current hunger — decreases passively over time
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float Hunger;

	// Upper bound for hunger; default 100
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float MaxHunger;

	// Current stamina — spent on resource collection, recovers passively
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float Stamina;

	// Upper bound for stamina; default 100
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float MaxStamina;

	// How many hunger units per second are lost (tune in Blueprint Details)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float HungerDecayRate;

	// How many health units per second are lost when Hunger == 0
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float HealthDecayRate;

	// How many stamina units per second are restored when not collecting
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	float StaminaRestoreRate;

	// Widget Blueprint to instantiate as the always-on stat HUD (assign WBP_PlayerStatHUD)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stats", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UPlayerStatWidget> PlayerStatWidgetClass;

	// The stat HUD widget instance; created once in BeginPlay and refreshed every tick
	UPROPERTY(Transient)
	UPlayerStatWidget* PlayerStatWidgetInstance;

	// ── Resource Inventory ────────────────────────────────────────────────────

	// Units of wood collected from trees
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 Wood;

	// Units of stone collected from rocks
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 Stone;

	// Units of berry collected from bushes
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Inventory", meta = (AllowPrivateAccess = "true"))
	int32 Berry;

	// ── Interaction ───────────────────────────────────────────────────────────

	// Maximum distance (cm) at which the player can interact with objects
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Interaction", meta = (AllowPrivateAccess = "true"))
	float InteractRange;

	// ── Building System ───────────────────────────────────────────────────────

	// Blueprint child of ABuildableObject to spawn when the player selects "Wall" (assign BP_Wall)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ABuildableObject> WallClass;

	// Blueprint child of ABuildableObject to spawn when the player selects "Floor" (assign BP_Floor)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ABuildableObject> FloorClass;

	// Blueprint child of ABuildableObject to spawn when the player selects "Ceiling" (assign BP_Ceiling)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<ABuildableObject> CeilingClass;

	// Widget Blueprint to instantiate as the build menu HUD (assign WBP_BuildMenu)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UBuildMenuWidget> BuildMenuWidgetClass;

	// Resources required to place one Wall
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building", meta = (AllowPrivateAccess = "true"))
	FBuildingCost WallCost;

	// Resources required to place one Floor
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building", meta = (AllowPrivateAccess = "true"))
	FBuildingCost FloorCost;

	// Resources required to place one Ceiling
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building", meta = (AllowPrivateAccess = "true"))
	FBuildingCost CeilingCost;

	// Maximum distance (cm) at which a shelter piece can be placed
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Building", meta = (AllowPrivateAccess = "true"))
	float BuildPlacementRange;

	// The build menu widget instance; created once in BeginPlay and shown/hidden by ToggleBuildMenu()
	UPROPERTY(Transient)
	UBuildMenuWidget* BuildMenuWidgetInstance;

	// Whether the build menu widget is currently visible
	bool bBuildMenuOpen;

	// Which piece the player is currently placing; None means no active placement
	EBuildPieceType SelectedBuildType;

	// The ghost/preview actor following the player's trace while placing (nullptr when not placing)
	UPROPERTY(Transient)
	ABuildableObject* PreviewActor;

	// Whether the last placement trace hit a valid surface; gates TryPlaceBuildable()
	bool bLastBuildTraceValid;

	// ── Objectives ────────────────────────────────────────────────────────────

	// Total Wood + Stone ever collected (lifetime count, not current inventory)
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Objectives", meta = (AllowPrivateAccess = "true"))
	int32 MaterialsCollected;

	// Target materials count for the "Collect materials" objective
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objectives", meta = (AllowPrivateAccess = "true"))
	int32 MaterialsGoal;

	// Total shelter pieces (Wall/Floor/Ceiling) successfully placed
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Objectives", meta = (AllowPrivateAccess = "true"))
	int32 PartsBuilt;

	// Target parts count for the "Build parts" objective
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objectives", meta = (AllowPrivateAccess = "true"))
	int32 PartsGoal;

	// Widget Blueprint to instantiate as the always-on objective HUD (assign WBP_ObjectiveHUD)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Objectives", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UObjectiveWidget> ObjectiveWidgetClass;

	// The objective HUD widget instance; created once in BeginPlay and refreshed whenever progress changes
	UPROPERTY(Transient)
	UObjectiveWidget* ObjectiveWidgetInstance;

	// ── Win/Lose ──────────────────────────────────────────────────────────────

	// How many seconds the player has to complete both objectives; tune in Blueprint Details
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameState", meta = (AllowPrivateAccess = "true"))
	float TimeLimit;

	// Counts down from TimeLimit each tick; reaching 0 without both objectives complete is a loss
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "GameState", meta = (AllowPrivateAccess = "true"))
	float TimeRemaining;

	// Widget Blueprint shown when both objectives are completed in time (assign WBP_Win)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameState", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UEndGameWidget> WinWidgetClass;

	// Widget Blueprint shown when time runs out or Health reaches 0 (assign WBP_Lose)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GameState", meta = (AllowPrivateAccess = "true"))
	TSubclassOf<UEndGameWidget> LoseWidgetClass;

	// True once the Win or Lose screen has been shown; stops further win/lose checks and stat ticking
	bool bGameOver;

	// ── Movement ──────────────────────────────────────────────────────────────

	// Top walking speed in cm/s; tunable in Blueprint Details
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Movement", meta = (AllowPrivateAccess = "true"))
	float WalkSpeed;

	// ── Private helpers ───────────────────────────────────────────────────────

	// Translates 2D move input into world-space movement each tick
	void Move(const FInputActionValue& Value);

	// Translates 2D look input into controller rotation each tick
	void Look(const FInputActionValue& Value);

	// Fires a line trace from the camera; calls Interact() on any hit interactable
	void TryInteract();

	// Opens/closes the build menu widget; cancels an active placement instead if one is in progress
	void ToggleBuildMenu();

	// Confirms placement of the current preview at its last valid trace location, if affordable
	void TryPlaceBuildable();

	// Moves/rotates the active preview actor to follow a trace from the camera each tick
	void UpdateBuildPreview();

	// Spawns a fresh ghost-preview actor of the given type at the player's location
	void SpawnPreviewActor(EBuildPieceType Type);

	// Destroys the active preview actor, if any
	void ClearPreviewActor();

	// Cancels the current placement and returns to the no-selection state
	void CancelBuildPlacement();

	// Pushes fresh inventory/cost text to the build menu widget, if it exists
	void RefreshBuildMenuDisplay();

	// Pushes fresh progress/completion state to the objective HUD widget, if it exists
	void RefreshObjectiveHUD();

	// Counts TimeRemaining down each tick; clamps to 0
	void UpdateGameTimer(float DeltaTime);

	// Shows WinWidgetClass, pauses the game, and frees the mouse cursor
	void TriggerWin();

	// Shows LoseWidgetClass, pauses the game, and frees the mouse cursor
	void TriggerLose();

	// Shared win/lose logic: creates WidgetClass, adds it to the viewport, and pauses the game
	void EndGame(TSubclassOf<UEndGameWidget> WidgetClass);

	// Maps a build piece type to its configured Blueprint class (WallClass/FloorClass/CeilingClass)
	TSubclassOf<ABuildableObject> GetBuildableClass(EBuildPieceType Type) const;

	// Decreases Hunger each frame; clamps to [0, MaxHunger]
	void UpdateHunger(float DeltaTime);

	// Drains Health each frame only when Hunger is zero; clamps to [0, MaxHealth]
	void UpdateHealth(float DeltaTime);

	// Passively restores Stamina each frame; clamps to [0, MaxStamina]
	void UpdateStamina(float DeltaTime);

public:
	// ── Public Interface (used by AInteractableObject subclasses) ─────────────

	/** Read-only access to current stamina so interactables can gate collection */
	float GetStamina() const { return Stamina; }

	/** Deduct Amount from Stamina; result is clamped to [0, MaxStamina] */
	void UseStamina(float Amount);

	/** Add Amount units of wood to the inventory */
	void AddWood(int32 Amount);

	/** Add Amount units of stone to the inventory */
	void AddStone(int32 Amount);

	/** Add Amount units of berry to the inventory */
	void AddBerry(int32 Amount);

	// ── Public Interface (used by UPlayerStatWidget) ───────────────────────────

	/** Current health as a 0..1 fraction, for the stat HUD progress bar */
	float GetHealthPercent() const { return MaxHealth > 0.f ? Health / MaxHealth : 0.f; }

	/** Current hunger as a 0..1 fraction, for the stat HUD progress bar */
	float GetHungerPercent() const { return MaxHunger > 0.f ? Hunger / MaxHunger : 0.f; }

	/** Current stamina as a 0..1 fraction, for the stat HUD progress bar */
	float GetStaminaPercent() const { return MaxStamina > 0.f ? Stamina / MaxStamina : 0.f; }

	// ── Public Interface (used by UBuildMenuWidget) ────────────────────────────

	/** Read-only access to current wood count for the build menu display */
	int32 GetWood() const { return Wood; }

	/** Read-only access to current stone count for the build menu display */
	int32 GetStone() const { return Stone; }

	/** Returns the configured resource cost for the given build piece type */
	FBuildingCost GetBuildCost(EBuildPieceType Type) const;

	/** Returns true if the current inventory covers the cost of the given build piece type */
	bool CanAffordBuild(EBuildPieceType Type) const;

	/**
	 * Called by UBuildMenuWidget when the player clicks a build button.
	 * Closes the menu, restores game input, and spawns a ghost preview of Type
	 * for the player to aim and place.
	 */
	void SelectBuildPiece(EBuildPieceType Type);

	// ── Public Interface (used by UObjectiveWidget) ────────────────────────────

	/** Lifetime Wood + Stone collected, for the "collect materials" objective */
	int32 GetMaterialsCollected() const { return MaterialsCollected; }

	/** Target materials count for the "collect materials" objective */
	int32 GetMaterialsGoal() const { return MaterialsGoal; }

	/** Total shelter pieces placed, for the "build parts" objective */
	int32 GetPartsBuilt() const { return PartsBuilt; }

	/** Target parts count for the "build parts" objective */
	int32 GetPartsGoal() const { return PartsGoal; }

	/** True once both the materials and parts objectives have been met */
	bool AreAllObjectivesComplete() const { return MaterialsCollected >= MaterialsGoal && PartsBuilt >= PartsGoal; }
};
