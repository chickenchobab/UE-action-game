// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Items/Item.h"
#include "Weapon.generated.h"

class USoundBase;
class UBoxComponent;
class ABaseCharacter;

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
	FORCEINLINE UBoxComponent* GetBox() { return WeaponBox; }
	FORCEINLINE void SetDamage(float DamageAmount) {Damage = DamageAmount;}
	FORCEINLINE void SetPair(AWeapon* Weapon) { PairWeapon = Weapon; }
	FORCEINLINE AWeapon* GetPair() { return PairWeapon; }
	
protected:
	virtual void BeginPlay() override;
	UFUNCTION()
  virtual void OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult);
  void TryApplyDamage(AActor *OtherActor);
  void ExecuteGetHit(AActor* OtherActor, FVector ImpactPoint);
  UFUNCTION(BlueprintImplementableEvent)
	void CreateFields(const FVector& FieldLocation);
	bool IsOwnerOpposite(AActor* OtherActor);

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* WeaponBox;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	float Damage = 20.f;

	TArray<AActor*> ActorsToIgnore;
	
private:
	bool IsActorIgnored(AActor* OtherActor);
	bool IsCharacterFacingWeapon(ABaseCharacter* HitCharacter);

	bool bMakeHitReaction = true;

	UPROPERTY(EditAnywhere, Category = "Weapon Properties")
	USoundBase *EquipSound;

	UPROPERTY(VisibleInstanceOnly)
	AWeapon* PairWeapon;
};


