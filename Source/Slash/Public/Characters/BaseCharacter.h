// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Interfaces/HitInterface.h"
#include "Characters/CharacterTypes.h"
#include "BaseCharacter.generated.h"

class AWeapon;
class UAttributeComponent;
class UAnimMontage;

UCLASS()
class SLASH_API ABaseCharacter : public ACharacter, public IHitInterface
{
	GENERATED_BODY()

public:
	ABaseCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter) override;
	bool IsAlive();
	bool IsOpposite(AActor* OtherActor);

	FORCEINLINE EDeathPose GetDeathPose() { return DeathPose; }
	
protected:
	virtual void BeginPlay() override;

	virtual void Attack();
	virtual void Die();
	UFUNCTION(BlueprintCallable)
	virtual void OnAttackEnded();
	UFUNCTION(BlueprintCallable)
	virtual void OnDodgeEnded();
	virtual bool CanAttack();
	virtual void HandleDamage(float DamageAmount);
	
	void SetCapsuleCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	
	UFUNCTION(BlueprintCallable)
	void SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled);
	
	void PlayMontageSection(UAnimMontage* Montage, const FName& SectionName);
	int32 PlayAttackMontage();
	int32 PlayDashAttackMontage();
	int32 PlayDeathMontage();
	void PlayDodgeMontage();
	void StopAttackMontage();
	void DirectionalHitReact(const FVector& ImpactPoint, const FVector& HitterLocation);

	void PlayHitSound(const FVector& ImpactPoint);
	void SpawnHitParticles(const FVector& ImpactPoint);

	bool IsFasterThan(float Speed);

	UPROPERTY(VisibleAnywhere)
	UAttributeComponent* Attributes;

	UPROPERTY(VisibleInstanceOnly, Category = Weapon)
	AWeapon* EquippedWeapon;

	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* AttackMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DashAttackMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* HitReactMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DeathMontage;
	UPROPERTY(EditDefaultsOnly, Category = Montages)
	UAnimMontage* DodgeMontage;

	UPROPERTY(BlueprintReadWrite)
	EDeathPose DeathPose;

	UPROPERTY(EditAnywhere)
	TArray<FName> AttackMontageSections;
	UPROPERTY(EditAnywhere)
	TArray<FName> DashAttackMontageSections;
	UPROPERTY(EditAnywhere)
	TArray<FName> DeathMontageSections;

private:
	int32 PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames);

	UPROPERTY(EditAnywhere, Category = Sounds)
	USoundBase* HitSound;

	UPROPERTY(EditAnywhere, Category = VisualEffects)
	UParticleSystem* HitParticles;
};


