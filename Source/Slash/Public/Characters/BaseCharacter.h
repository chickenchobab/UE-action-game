// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "BaseCharacter.generated.h"

class UAttributeComponent;
class AWeapon;
class AMeleeWeapon;
class ARangedWeapon;
class UAnimMontage;
class UBoxComponent;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter, bool bReact) override;
	bool IsAlive();
	bool IsOpposite(AActor* OtherActor);
	FORCEINLINE EDeathPose GetDeathPose() { return DeathPose; }
	FORCEINLINE virtual bool IsParrying() { return false; }
	
protected:
	virtual void Attack();
	virtual void Die();
	virtual void Parry();
	virtual void Dodge();
	UFUNCTION(BlueprintCallable)
	virtual void AttackEnd();
	UFUNCTION(BlueprintCallable)
	virtual void DodgeEnd();
	virtual bool CanAttack();
	virtual void HandleDamage(float DamageAmount);
	virtual void SpawnProjectile();
	UFUNCTION(BlueprintCallable)
	virtual void FireProjectile();
	// Mesh-specific function
	virtual void RotateProjectile(ARangedWeapon* Projectile);
	
	void SetCapsuleCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled, bool bPairWeapon = false);
	UFUNCTION(BlueprintCallable)
	void SetBodyCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	void PlayMontage(UAnimMontage* Montage);
	void StopMontage(float InBlendOutTime, UAnimMontage* Montage);
	void PauseMontage(UAnimMontage* Montage);
	void ResumeMontage(UAnimMontage* Montage);

	int32 PlayAttackMontage(bool bStartCombo = false);
	int32 PlaySpecialAttackMontage();
	int32 PlayDeathMontage();
	int32 PlayDodgeMontage();
	void StopAttackMontage(float InBlendOutTime = 0.25f);
	void DirectionalHitReact(const FVector& ImpactPoint, const FVector& HitterLocation);

	void PlaySound(const FVector& ImpactPoint, USoundBase* PlayedSound);
	void SpawnParticles(const FVector& ImpactPoint, UParticleSystem* SpawnedParticles);

	void CreateHandFootBox(FName BoxName, FName SocketName);
	void SetupHandFoot(int32 BodyIndex, float Damage, FName SocketName);

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY(VisibleInstanceOnly, Category = Weapon)
	AWeapon* EquippedWeapon;
	UPROPERTY(VisibleInstanceOnly, Category = Weapon)
	TArray<AWeapon*> HandsAndFeet;
	UPROPERTY(VisibleInstanceOnly, Category = Weapon)
	TArray<UBoxComponent*> HandFootBoxes;

	float BodyAttackDamage = 5.f;
	
	bool bBodyBlocked = false;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* SpecialAttackMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DodgeMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* ParryMontage;

	UPROPERTY(BlueprintReadWrite)
	EDeathPose DeathPose;

	UPROPERTY(EditAnywhere)
	TArray<FName> AttackMontageSections;
	UPROPERTY(EditAnywhere)
	TArray<FName> SpecialAttackMontageSections;
	UPROPERTY(EditAnywhere)
	TArray<FName> DeathMontageSections;
	UPROPERTY(EditAnywhere)
	TArray<FName> DodgeMontageSections;

private:
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundBase* HitSound;
	UPROPERTY(EditAnywhere, Category = VisualEffects)
	UParticleSystem* HitParticles;

	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundBase* ParrySound;
	UPROPERTY(EditAnywhere, Category = VisualEffects)
	UParticleSystem* ParryParticles;
};

