// Copyright Epic Games, Inc. All Rights Reserved.

#include "ObjectiveWidget.h"
#include "SurvivalCharacter.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void UObjectiveWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Fixed label text and starting hidden state; UpdateObjectives() only toggles visibility afterward
	if (CompletionText)
	{
		CompletionText->SetText(FText::FromString(TEXT("All Objectives Complete!")));
		CompletionText->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UObjectiveWidget::UpdateObjectives()
{
	if (!OwningCharacter)
	{
		return;
	}

	const int32 Materials     = OwningCharacter->GetMaterialsCollected();
	const int32 MaterialsGoal = OwningCharacter->GetMaterialsGoal();
	const int32 Parts         = OwningCharacter->GetPartsBuilt();
	const int32 PartsGoal     = OwningCharacter->GetPartsGoal();

	if (MaterialsBar)
	{
		const float Percent = MaterialsGoal > 0 ? static_cast<float>(Materials) / MaterialsGoal : 0.f;
		MaterialsBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
	if (MaterialsText)
	{
		MaterialsText->SetText(FText::FromString(FString::Printf(TEXT("Materials: %d / %d"), Materials, MaterialsGoal)));
	}

	if (PartsBar)
	{
		const float Percent = PartsGoal > 0 ? static_cast<float>(Parts) / PartsGoal : 0.f;
		PartsBar->SetPercent(FMath::Clamp(Percent, 0.f, 1.f));
	}
	if (PartsText)
	{
		PartsText->SetText(FText::FromString(FString::Printf(TEXT("Parts Built: %d / %d"), Parts, PartsGoal)));
	}

	// Only reveal the banner once both objectives are simultaneously satisfied
	if (CompletionText)
	{
		CompletionText->SetVisibility(OwningCharacter->AreAllObjectivesComplete()
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);
	}
}
