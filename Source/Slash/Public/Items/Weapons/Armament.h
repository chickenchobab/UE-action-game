// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Weapons/Weapon.h"
#include "Armament.generated.h"

/**
 * 
 */
UCLASS()
class SLASH_API AArmament : public AWeapon
{
	GENERATED_BODY()

public:
	AArmament();

protected:
	virtual void BeginPlay();
  virtual void OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult) override;
	
private:
	void BoxTrace(FHitResult& BoxHit);
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceStart;
	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceEnd;

	bool bShowDebugBox = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FVector BoxTraceExtent = FVector(5.f);
};
