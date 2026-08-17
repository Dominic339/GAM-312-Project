// Copyright Epic Games, Inc. All Rights Reserved.

#include "EndGameWidget.h"
#include "Components/Button.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

void UEndGameWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// Wire both buttons here so WBP_Win/WBP_Lose need no graph logic
	if (RestartButton)
	{
		RestartButton->OnClicked.AddDynamic(this, &UEndGameWidget::OnRestartClicked);
	}
	if (QuitButton)
	{
		QuitButton->OnClicked.AddDynamic(this, &UEndGameWidget::OnQuitClicked);
	}
}

void UEndGameWidget::OnRestartClicked()
{
	// Reloading the current level resets every actor, stat, and objective back to its defaults
	UGameplayStatics::OpenLevel(this, FName(*GetWorld()->GetName()));
}

void UEndGameWidget::OnQuitClicked()
{
	APlayerController* PC = GetOwningPlayer();
	UKismetSystemLibrary::QuitGame(this, PC, EQuitPreference::Quit, false);
}
