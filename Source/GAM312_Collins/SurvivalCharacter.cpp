// Copyright Epic Games, Inc. All Rights Reserved.

#include "SurvivalCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InteractableObject.h"
#include "BuildableObject.h"
#include "BuildMenuWidget.h"
#include "PlayerStatWidget.h"
#include "ObjectiveWidget.h"
#include "Blueprint/UserWidget.h"
#include "DrawDebugHelpers.h"

ASurvivalCharacter::ASurvivalCharacter()
{
	// We need Tick so that stats update every frame
	PrimaryActorTick.bCanEverTick = true;

	// ── Camera ───────────────────────────────────────────────────────────────
	// Attach the camera at a standard eye height (64 units ≈ 170 cm scaled)
	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetRootComponent());
	FirstPersonCamera->SetRelativeLocation(FVector(0.f, 0.f, 64.f));

	// Camera follows controller rotation so mouse input directly aims the view
	FirstPersonCamera->bUsePawnControlRotation = true;

	// Yaw rotates the whole character so movement always goes where we're looking
	bUseControllerRotationYaw   = true;
	// Pitch and roll only affect the camera, not the capsule
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll  = false;

	// ── Movement ─────────────────────────────────────────────────────────────
	WalkSpeed = 600.f;

	// ── Stats — start fully stocked ──────────────────────────────────────────
	MaxHealth  = 100.f;   Health  = MaxHealth;
	MaxHunger  = 100.f;   Hunger  = MaxHunger;
	MaxStamina = 100.f;   Stamina = MaxStamina;

	// Hunger empties in ~50 s at default; tune this in the Blueprint Details panel
	HungerDecayRate    = 2.f;
	// Health drains at 5/s once starving — player has ~20 s to find food
	HealthDecayRate    = 5.f;
	// Stamina recovers at 10/s — player recovers fully in 10 s of not collecting
	StaminaRestoreRate = 10.f;

	// ── Inventory — start empty ──────────────────────────────────────────────
	Wood  = 0;
	Stone = 0;
	Berry = 0;

	// ── Interaction ──────────────────────────────────────────────────────────
	// 300 cm (3 m) — close enough to feel physical, far enough to be comfortable
	InteractRange = 300.f;

	// ── Building System ──────────────────────────────────────────────────────
	// 500 cm (5 m) — a bit further than interaction range so shelters can be placed at arm's length
	BuildPlacementRange = 500.f;

	// Default costs; tune per-type in the Blueprint Details panel
	WallCost.WoodCost    = 3;
	WallCost.StoneCost   = 1;
	FloorCost.WoodCost   = 4;
	FloorCost.StoneCost  = 0;
	CeilingCost.WoodCost = 3;
	CeilingCost.StoneCost = 2;

	BuildMenuWidgetInstance = nullptr;
	bBuildMenuOpen          = false;
	SelectedBuildType       = EBuildPieceType::None;
	PreviewActor            = nullptr;
	bLastBuildTraceValid    = false;

	PlayerStatWidgetInstance = nullptr;

	// ── Objectives ────────────────────────────────────────────────────────────
	MaterialsCollected = 0;
	MaterialsGoal      = 500;
	PartsBuilt         = 0;
	PartsGoal          = 5;
	ObjectiveWidgetInstance = nullptr;
}

// ─────────────────────────────────────────────────────────────────────────────
// BeginPlay
// ─────────────────────────────────────────────────────────────────────────────

void ASurvivalCharacter::BeginPlay()
{
	Super::BeginPlay();

	// Apply the configured walk speed to the movement component
	GetCharacterMovement()->MaxWalkSpeed = WalkSpeed;

	// Register the mapping context so the engine resolves key→action bindings
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem =
			ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			// Priority 0 — our only active context
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		// Create the build menu widget once and keep it hidden until the player opens it
		if (BuildMenuWidgetClass)
		{
			BuildMenuWidgetInstance = CreateWidget<UBuildMenuWidget>(PC, BuildMenuWidgetClass);
			if (BuildMenuWidgetInstance)
			{
				BuildMenuWidgetInstance->OwningCharacter = this;
				BuildMenuWidgetInstance->AddToViewport();
				BuildMenuWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
			}
		}

		// Create the stat HUD and leave it visible for the whole match — Tick() keeps it refreshed
		if (PlayerStatWidgetClass)
		{
			PlayerStatWidgetInstance = CreateWidget<UPlayerStatWidget>(PC, PlayerStatWidgetClass);
			if (PlayerStatWidgetInstance)
			{
				PlayerStatWidgetInstance->OwningCharacter = this;
				PlayerStatWidgetInstance->AddToViewport();
				PlayerStatWidgetInstance->UpdateStats();
			}
		}

		// Create the objective HUD and leave it visible for the whole match
		if (ObjectiveWidgetClass)
		{
			ObjectiveWidgetInstance = CreateWidget<UObjectiveWidget>(PC, ObjectiveWidgetClass);
			if (ObjectiveWidgetInstance)
			{
				ObjectiveWidgetInstance->OwningCharacter = this;
				ObjectiveWidgetInstance->AddToViewport();
				ObjectiveWidgetInstance->UpdateObjectives();
			}
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Tick — update all passive stats every frame
// ─────────────────────────────────────────────────────────────────────────────

void ASurvivalCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Order matters: hunger falls first, then health responds to hunger being zero,
	// then stamina recovers independently of the other two.
	UpdateHunger(DeltaTime);
	UpdateHealth(DeltaTime);
	UpdateStamina(DeltaTime);

	// Refresh the stat HUD every tick so its progress bars track the stats above in real time
	if (PlayerStatWidgetInstance)
	{
		PlayerStatWidgetInstance->UpdateStats();
	}

	// While a piece is selected, keep its ghost preview following the player's aim
	if (SelectedBuildType != EBuildPieceType::None)
	{
		UpdateBuildPreview();
	}
}

void ASurvivalCharacter::UpdateHunger(float DeltaTime)
{
	// Reduce hunger by the decay rate scaled to elapsed time
	Hunger -= HungerDecayRate * DeltaTime;

	// Clamp so it never goes below zero or above the maximum
	Hunger = FMath::Clamp(Hunger, 0.f, MaxHunger);
}

void ASurvivalCharacter::UpdateHealth(float DeltaTime)
{
	// Health only drains when the player is fully starved
	if (Hunger <= 0.f)
	{
		Health -= HealthDecayRate * DeltaTime;
		Health = FMath::Clamp(Health, 0.f, MaxHealth);
	}
}

void ASurvivalCharacter::UpdateStamina(float DeltaTime)
{
	// Stamina restores passively every frame regardless of hunger or health
	Stamina += StaminaRestoreRate * DeltaTime;
	Stamina = FMath::Clamp(Stamina, 0.f, MaxStamina);
}

// ─────────────────────────────────────────────────────────────────────────────
// Input setup
// ─────────────────────────────────────────────────────────────────────────────

void ASurvivalCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EIC = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Triggered fires every tick a key is held — needed for smooth movement
		EIC->BindAction(MoveAction,     ETriggerEvent::Triggered, this, &ASurvivalCharacter::Move);
		EIC->BindAction(LookAction,     ETriggerEvent::Triggered, this, &ASurvivalCharacter::Look);

		// Started fires once per key press — single interaction per press is intentional
		EIC->BindAction(InteractAction, ETriggerEvent::Started,   this, &ASurvivalCharacter::TryInteract);

		// Started fires once per key press — opens/closes the build menu, or cancels a placement in progress
		EIC->BindAction(ToggleBuildMenuAction, ETriggerEvent::Started, this, &ASurvivalCharacter::ToggleBuildMenu);

		// Started fires once per press — confirms placement of the currently previewed piece
		EIC->BindAction(PlaceBuildableAction,  ETriggerEvent::Started, this, &ASurvivalCharacter::TryPlaceBuildable);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Movement handlers
// ─────────────────────────────────────────────────────────────────────────────

void ASurvivalCharacter::Move(const FInputActionValue& Value)
{
	// MoveAction value type must be Axis2D: X = strafe, Y = forward/back
	const FVector2D MovementInput = Value.Get<FVector2D>();

	if (Controller)
	{
		AddMovementInput(GetActorForwardVector(), MovementInput.Y);
		AddMovementInput(GetActorRightVector(),   MovementInput.X);
	}
}

void ASurvivalCharacter::Look(const FInputActionValue& Value)
{
	// LookAction value type must be Axis2D: X = yaw, Y = pitch
	const FVector2D LookInput = Value.Get<FVector2D>();

	if (Controller)
	{
		AddControllerYawInput(LookInput.X);
		AddControllerPitchInput(LookInput.Y);
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Interaction — line trace from camera forward
// ─────────────────────────────────────────────────────────────────────────────

void ASurvivalCharacter::TryInteract()
{
	// Build trace ray: starts at the camera (player's eyes), ends at InteractRange
	const FVector TraceStart = FirstPersonCamera->GetComponentLocation();
	const FVector TraceEnd   = TraceStart + (FirstPersonCamera->GetForwardVector() * InteractRange);

	// Draw a short debug line in the editor so you can see where the trace goes
	// (magenta line, visible for 2 seconds — harmless in PIE, invisible in packaged builds)
	DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Magenta, false, 2.f, 0, 1.5f);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;

	// Ignore the player's own capsule so we don't immediately hit ourselves
	QueryParams.AddIgnoredActor(this);

	// Fire the trace on the Visibility channel — interactable meshes must block this channel
	const bool bHit = GetWorld()->LineTraceSingleByChannel(
		HitResult,
		TraceStart,
		TraceEnd,
		ECC_Visibility,
		QueryParams
	);

	if (bHit)
	{
		// Cast the hit actor to our base interactable type
		if (AInteractableObject* Interactable = Cast<AInteractableObject>(HitResult.GetActor()))
		{
			// Delegate all resource-award and depletion logic to the object itself
			Interactable->Interact(this);
		}
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Public interface — called by AInteractableObject subclasses
// ─────────────────────────────────────────────────────────────────────────────

void ASurvivalCharacter::UseStamina(float Amount)
{
	// Clamp so we never store negative stamina
	Stamina = FMath::Clamp(Stamina - Amount, 0.f, MaxStamina);
}

void ASurvivalCharacter::AddWood(int32 Amount)
{
	// Add wood to the inventory; negative amounts are intentionally rejected
	if (Amount > 0)
	{
		Wood += Amount;

		// Wood counts toward the "collect materials" objective; Berry does not (it's food, not a building material)
		MaterialsCollected += Amount;

		RefreshBuildMenuDisplay();
		RefreshObjectiveHUD();
	}
}

void ASurvivalCharacter::AddStone(int32 Amount)
{
	// Add stone to the inventory; negative amounts are intentionally rejected
	if (Amount > 0)
	{
		Stone += Amount;
		MaterialsCollected += Amount;

		RefreshBuildMenuDisplay();
		RefreshObjectiveHUD();
	}
}

void ASurvivalCharacter::AddBerry(int32 Amount)
{
	// Add berries to the inventory; negative amounts are intentionally rejected
	if (Amount > 0)
	{
		Berry += Amount;
	}
}

// ─────────────────────────────────────────────────────────────────────────────
// Building system
// ─────────────────────────────────────────────────────────────────────────────

void ASurvivalCharacter::ToggleBuildMenu()
{
	// If the player is mid-placement, this key cancels that instead of toggling the menu
	if (SelectedBuildType != EBuildPieceType::None)
	{
		CancelBuildPlacement();
		return;
	}

	if (!BuildMenuWidgetInstance)
	{
		return;
	}

	bBuildMenuOpen = !bBuildMenuOpen;

	APlayerController* PC = Cast<APlayerController>(GetController());

	if (bBuildMenuOpen)
	{
		// Refresh costs/inventory right before showing so the display is never stale
		RefreshBuildMenuDisplay();
		BuildMenuWidgetInstance->SetVisibility(ESlateVisibility::Visible);

		if (PC)
		{
			// Show the cursor and let clicks reach the widget while keeping the game world ticking
			PC->SetInputMode(FInputModeGameAndUI());
			PC->bShowMouseCursor = true;
		}
	}
	else
	{
		BuildMenuWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);

		if (PC)
		{
			PC->SetInputMode(FInputModeGameOnly());
			PC->bShowMouseCursor = false;
		}
	}
}

void ASurvivalCharacter::SelectBuildPiece(EBuildPieceType Type)
{
	// Close the menu and hand control back to the game world so the player can aim the preview
	bBuildMenuOpen = false;
	if (BuildMenuWidgetInstance)
	{
		BuildMenuWidgetInstance->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		PC->SetInputMode(FInputModeGameOnly());
		PC->bShowMouseCursor = false;
	}

	SelectedBuildType = Type;
	SpawnPreviewActor(Type);
}

void ASurvivalCharacter::SpawnPreviewActor(EBuildPieceType Type)
{
	// Replace any existing preview — only one piece can be aimed at a time
	ClearPreviewActor();

	const TSubclassOf<ABuildableObject> ClassToSpawn = GetBuildableClass(Type);
	if (!ClassToSpawn)
	{
		return;
	}

	FActorSpawnParameters SpawnParams;
	// The preview has no collision yet, but AlwaysSpawn avoids any edge-case spawn failures
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	PreviewActor = GetWorld()->SpawnActor<ABuildableObject>(ClassToSpawn, GetActorLocation(), FRotator::ZeroRotator, SpawnParams);
	if (PreviewActor)
	{
		PreviewActor->SetPreviewMode(true);
	}
}

void ASurvivalCharacter::ClearPreviewActor()
{
	if (PreviewActor)
	{
		PreviewActor->Destroy();
		PreviewActor = nullptr;
	}
}

void ASurvivalCharacter::CancelBuildPlacement()
{
	ClearPreviewActor();
	SelectedBuildType    = EBuildPieceType::None;
	bLastBuildTraceValid = false;
}

void ASurvivalCharacter::UpdateBuildPreview()
{
	if (!PreviewActor)
	{
		return;
	}

	// Same trace-from-camera approach as TryInteract, but at building range instead of interact range
	const FVector TraceStart = FirstPersonCamera->GetComponentLocation();
	const FVector TraceEnd   = TraceStart + (FirstPersonCamera->GetForwardVector() * BuildPlacementRange);

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);
	QueryParams.AddIgnoredActor(PreviewActor);

	bLastBuildTraceValid = GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, QueryParams);

	if (bLastBuildTraceValid)
	{
		// Snap the ghost to the hit point and orient it to the surface it's resting against
		PreviewActor->SetActorLocation(HitResult.Location);
		PreviewActor->SetActorRotation(HitResult.Normal.Rotation());
	}
}

void ASurvivalCharacter::TryPlaceBuildable()
{
	// Nothing to place if no piece is selected, no preview exists, or the last trace missed
	if (SelectedBuildType == EBuildPieceType::None || !PreviewActor || !bLastBuildTraceValid)
	{
		return;
	}

	if (!CanAffordBuild(SelectedBuildType))
	{
		// Not enough resources yet — leave the preview active so the player can go collect more
		return;
	}

	const FBuildingCost Cost = GetBuildCost(SelectedBuildType);
	Wood  -= Cost.WoodCost;
	Stone -= Cost.StoneCost;

	// Lock the preview in as a real, fully-collidable piece of the shelter
	PreviewActor->SetPreviewMode(false);
	PreviewActor = nullptr;

	// Every successfully placed piece counts toward the "build parts" objective
	PartsBuilt += 1;

	RefreshBuildMenuDisplay();
	RefreshObjectiveHUD();

	// Spawn a fresh preview of the same type so the player can keep building without reopening the menu
	SpawnPreviewActor(SelectedBuildType);
}

void ASurvivalCharacter::RefreshBuildMenuDisplay()
{
	if (BuildMenuWidgetInstance)
	{
		BuildMenuWidgetInstance->UpdateInventoryDisplay();
	}
}

void ASurvivalCharacter::RefreshObjectiveHUD()
{
	if (ObjectiveWidgetInstance)
	{
		ObjectiveWidgetInstance->UpdateObjectives();
	}
}

TSubclassOf<ABuildableObject> ASurvivalCharacter::GetBuildableClass(EBuildPieceType Type) const
{
	switch (Type)
	{
		case EBuildPieceType::Wall:    return WallClass;
		case EBuildPieceType::Floor:   return FloorClass;
		case EBuildPieceType::Ceiling: return CeilingClass;
		default:                       return nullptr;
	}
}

FBuildingCost ASurvivalCharacter::GetBuildCost(EBuildPieceType Type) const
{
	switch (Type)
	{
		case EBuildPieceType::Wall:    return WallCost;
		case EBuildPieceType::Floor:   return FloorCost;
		case EBuildPieceType::Ceiling: return CeilingCost;
		default:                       return FBuildingCost();
	}
}

bool ASurvivalCharacter::CanAffordBuild(EBuildPieceType Type) const
{
	const FBuildingCost Cost = GetBuildCost(Type);
	return Wood >= Cost.WoodCost && Stone >= Cost.StoneCost;
}
