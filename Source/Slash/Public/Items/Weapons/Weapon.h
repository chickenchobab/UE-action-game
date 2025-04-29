// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

class USoundBase;
class UBoxComponent;

/**
 * 
 */
UCLASS()
class SLASH_API AWeapon : public AItem
{
	GENERATED_BODY()

public:
	AWeapon();

	void Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator, bool bPlayEquipSound = false);
	void AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName);
	void SetWeaponBoxCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	void ResetActorsToIgnore();
	
protected:
	virtual void BeginPlay() override;
	
	UFUNCTION()
  void OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	
  UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);
	
private:
	void BoxTrace(FHitResult& BoxHit);
  void ExecuteGetHit(FHitResult &BoxHit);
	bool IsOwnerOpposite(AActor* OtherActor);

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* WeaponBox;
	
	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceStart;
	UPROPERTY(VisibleAnywhere)
	USceneComponent* BoxTraceEnd;
	
	TArray<AActor*> ActorsToIgnore;

	bool bShowDebugBox = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	FVector BoxTraceExtent = FVector(5.f);

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	USoundBase *EquipSound;
	
	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Damage = 20.f;
};
