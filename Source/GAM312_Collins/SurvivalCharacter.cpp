// Copyright Epic Games, Inc. All Rights Reserved.

#include "SurvivalCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InteractableObject.h"
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
	}
}

void ASurvivalCharacter::AddStone(int32 Amount)
{
	// Add stone to the inventory; negative amounts are intentionally rejected
	if (Amount > 0)
	{
		Stone += Amount;
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
