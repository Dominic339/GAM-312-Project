// Copyright Epic Games, Inc. All Rights Reserved.

#include "PlayerStatWidget.h"
#include "SurvivalCharacter.h"
#include "Components/ProgressBar.h"

void UPlayerStatWidget::UpdateStats()
{
	if (!OwningCharacter)
	{
		return;
	}

	// UProgressBar::Percent expects a 0..1 fraction, which the character already computes
	if (HealthBar)
	{
		HealthBar->SetPercent(OwningCharacter->GetHealthPercent());
	}
	if (HungerBar)
	{
		HungerBar->SetPercent(OwningCharacter->GetHungerPercent());
	}
	if (StaminaBar)
	{
		StaminaBar->SetPercent(OwningCharacter->GetStaminaPercent());
	}
}
