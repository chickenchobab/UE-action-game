// Fill out your copyright notice in the Description page of Project Settings.


#include "HUD/SlashOverlay.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"

void USlashOverlay::SetHealthBarPercent(float Percent)
{
  if (HealthBar)
  {
    HealthBar->SetPercent(Percent);
  }
}

void USlashOverlay::SetStaminaBarPercent(float Percent)
{
  if (StaminaBar)
  {
    StaminaBar->SetPercent(Percent);
  }
}

void USlashOverlay::SetGold(int32 Gold)
{
	if (GoldText)
  {
    const FString GoldString = FString::Printf(TEXT("%d"), Gold);
    GoldText->SetText(FText::FromString(GoldString));
  }
}

void USlashOverlay::SetSouls(int32 Soul)
{
	if (SoulText)
  {
    const FString SoulString = FString::Printf(TEXT("%d"), Soul);
    SoulText->SetText(FText::FromString(SoulString));
  }
}

