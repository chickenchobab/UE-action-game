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
	void DetachMeshFromSocket();
	void SetWeaponBoxCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	void ResetActorsToIgnore();

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void SetBlocked(bool bWeaponBlocked) { bBlocked = bWeaponBlocked; }
	FORCEINLINE bool IsBlocked() { return bBlocked; }
	
protected:
	UFUNCTION()
  virtual void OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
	void ExecuteGetHit(const FHitResult &HitResult);
  UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);
	bool IsOwnerOpposite(AActor* OtherActor);

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* WeaponBox;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Damage = 20.f;

	TArray<AActor*> ActorsToIgnore;
	
private:
	bool bBlocked = false;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	USoundBase *EquipSound;
};

