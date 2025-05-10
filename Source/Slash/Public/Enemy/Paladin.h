// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Enemy/Enemy.h"
#include "Paladin.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API APaladin : public AEnemy
{
	GENERATED_BODY()

public:
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void BeginPlay() override;

	virtual void Attack() override;
	virtual bool CanAttack() override;
	
	virtual void CheckCombatTarget() override;
};
