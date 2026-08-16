// Copyright Epic Games, Inc. All Rights Reserved.

#include "BuildMenuWidget.h"
#include "SurvivalCharacter.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UBuildMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Wire every button's click event here so WBP_BuildMenu needs no graph logic
	if (BuildWallButton)
	{
		BuildWallButton->OnClicked.AddDynamic(this, &UBuildMenuWidget::OnBuildWallClicked);
	}
	if (BuildFloorButton)
	{
		BuildFloorButton->OnClicked.AddDynamic(this, &UBuildMenuWidget::OnBuildFloorClicked);
	}
	if (BuildCeilingButton)
	{
		BuildCeilingButton->OnClicked.AddDynamic(this, &UBuildMenuWidget::OnBuildCeilingClicked);
	}
}

void UBuildMenuWidget::UpdateInventoryDisplay()
{
	if (!OwningCharacter)
	{
		return;
	}

	if (WoodCountText)
	{
		WoodCountText->SetText(FText::AsNumber(OwningCharacter->GetWood()));
	}
	if (StoneCountText)
	{
		StoneCountText->SetText(FText::AsNumber(OwningCharacter->GetStone()));
	}

	const FBuildingCost WallCost    = OwningCharacter->GetBuildCost(EBuildPieceType::Wall);
	const FBuildingCost FloorCost   = OwningCharacter->GetBuildCost(EBuildPieceType::Floor);
	const FBuildingCost CeilingCost = OwningCharacter->GetBuildCost(EBuildPieceType::Ceiling);

	if (WallCostText)
	{
		WallCostText->SetText(FText::FromString(FString::Printf(TEXT("Wood: %d  Stone: %d"), WallCost.WoodCost, WallCost.StoneCost)));
	}
	if (FloorCostText)
	{
		FloorCostText->SetText(FText::FromString(FString::Printf(TEXT("Wood: %d  Stone: %d"), FloorCost.WoodCost, FloorCost.StoneCost)));
	}
	if (CeilingCostText)
	{
		CeilingCostText->SetText(FText::FromString(FString::Printf(TEXT("Wood: %d  Stone: %d"), CeilingCost.WoodCost, CeilingCost.StoneCost)));
	}
}

void UBuildMenuWidget::OnBuildWallClicked()
{
	if (OwningCharacter)
	{
		OwningCharacter->SelectBuildPiece(EBuildPieceType::Wall);
	}
}

void UBuildMenuWidget::OnBuildFloorClicked()
{
	if (OwningCharacter)
	{
		OwningCharacter->SelectBuildPiece(EBuildPieceType::Floor);
	}
}

void UBuildMenuWidget::OnBuildCeilingClicked()
{
	if (OwningCharacter)
	{
		OwningCharacter->SelectBuildPiece(EBuildPieceType::Ceiling);
	}
}
