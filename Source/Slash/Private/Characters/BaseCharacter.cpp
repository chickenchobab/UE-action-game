// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Items/Weapons/RangedWeapon.h"
#include "Components/BoxComponent.h"
#include "Components/AttributeComponent.h"
#include "Kismet/GameplayStatics.h"


ABaseCharacter::ABaseCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABaseCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter)
{
	if (IsAlive() && IsParrying())
	{
		PlaySound(ImpactPoint, ParrySound);
		SpawnParticles(ImpactPoint, ParryParticles);
		return;
	}

	PlaySound(ImpactPoint, HitSound);
	SpawnParticles(ImpactPoint, HitParticles);

	if (IsAlive())
	{
		DirectionalHitReact(ImpactPoint, Hitter->GetActorLocation());
	}
	else
	{
		Die();
	}
}

bool ABaseCharacter::IsAlive()
{
	return Attributes && Attributes->IsAlive();
}


bool ABaseCharacter::IsOpposite(AActor* OtherActor)
{
	if (OtherActor == nullptr) return false;
	if (Tags.IsEmpty() || OtherActor->Tags.IsEmpty()) return true;
  return Tags[0] != OtherActor->Tags[0];
}


void ABaseCharacter::Attack()
{
	
}

void ABaseCharacter::Die()
{
	Tags.Add("Dead");
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
    AnimInstance->Montage_Stop(0.1f);
	}
	PlayDeathMontage();
}

void ABaseCharacter::Parry()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && ParryMontage)
	{
		AnimInstance->Montage_Play(ParryMontage);
		AnimInstance->Montage_Pause(ParryMontage);
	}
}

void ABaseCharacter::AttackEnd()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->SetBlocked(false);
	}
	for (int i = 0; i < HandsAndFeet.Num(); ++i)
	{
		if (HandsAndFeet[i])
		{
			HandsAndFeet[i]->SetBlocked(false);
		}
	}
}

void ABaseCharacter::DodgeEnd()
{
	
}

bool ABaseCharacter::CanAttack()
{
	return true;
}

void ABaseCharacter::HandleDamage(float DamageAmount)
{
	if (Attributes)
	{
		Attributes->ReceiveDamage(DamageAmount);
	}
}



void ABaseCharacter::SpawnProjectile()
{
	
}

void ABaseCharacter::FireProjectile()
{
	
}

void ABaseCharacter::SetCapsuleCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	GetCapsuleComponent()->SetCollisionEnabled(CollisionEnabled);
}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled, bool bPairWeapon)
{
	if (EquippedWeapon == nullptr) return;

	if (bPairWeapon)
	{
		if (AWeapon* PairWeapon = EquippedWeapon->GetPair())
		{
			PairWeapon->SetWeaponBoxCollisionEnabled(CollisionEnabled);
			PairWeapon->ResetActorsToIgnore();
		}
	}
	else 
	{
		EquippedWeapon->SetWeaponBoxCollisionEnabled(CollisionEnabled);
		EquippedWeapon->ResetActorsToIgnore();
	}
}


void ABaseCharacter::SetBodyCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	for (int i = 0; i < HandsAndFeet.Num(); ++i)
	{
		if (HandsAndFeet[i])
		{
			HandsAndFeet[i]->SetWeaponBoxCollisionEnabled(CollisionEnabled);
			HandsAndFeet[i]->ResetActorsToIgnore();
		}
	}
}


void ABaseCharacter::PlayMontageSection(UAnimMontage* Montage, const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && Montage)
	{
		AnimInstance->Montage_Play(Montage);
		AnimInstance->Montage_JumpToSection(SectionName, Montage);
	}
}

int32 ABaseCharacter::PlayAttackMontage(bool bStartCombo)
{
	if (bStartCombo && !AttackMontageSections.IsEmpty())
	{
		PlayMontageSection(AttackMontage, AttackMontageSections[0]);
		return 0;
	}
	return PlayRandomMontageSection(AttackMontage, AttackMontageSections);
}


int32 ABaseCharacter::PlaySpecialAttackMontage()
{
	return PlayRandomMontageSection(SpecialAttackMontage, SpecialAttackMontageSections);
}


int32 ABaseCharacter::PlayDeathMontage()
{
	int32 Selection = PlayRandomMontageSection(DeathMontage, DeathMontageSections);
	if (Selection < 0) return -1;
	
	EDeathPose Pose = static_cast<EDeathPose>(Selection);
	if (Pose >= EDeathPose::EDP_MAX) return -1;

	DeathPose = Pose; // It is used by animation blueprint
	return Selection;
}

int32 ABaseCharacter::PlayDodgeMontage()
{
	return PlayRandomMontageSection(DodgeMontage, DodgeMontageSections);
}

void ABaseCharacter::StopAttackMontage(float InBlendOutTime)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Stop(InBlendOutTime, AttackMontage);
	}
}


void ABaseCharacter::DirectionalHitReact(const FVector& ImpactPoint, const FVector& HitterLocation)
{
	const FVector Forward = GetActorForwardVector();
	const FVector HitterLocationParallel(HitterLocation.X, HitterLocation.Y, GetActorLocation().Z);
	const FVector ToHitter = (HitterLocationParallel - GetActorLocation()).GetSafeNormal();

	double Theta = FMath::Acos(FVector::DotProduct(Forward, ToHitter));
	Theta = FMath::RadiansToDegrees(Theta);

	const FVector CrossProduct = FVector::CrossProduct(Forward, ToHitter);
	if (CrossProduct.Z < 0)
	{
		Theta *= -1;
	}

	FName Section("FromBack");
	if (Theta >= -45.f && Theta < 45.f)
	{
		Section = FName("FromFront");
	}
	else if (Theta >= -135.f && Theta < -45.f)
	{
		Section = FName("FromLeft");
	}
	else if (Theta >= 45.f && Theta < 135.f)
	{
		Section = FName("FromRight");
	}

	PlayMontageSection(HitReactMontage, Section);
}



void ABaseCharacter::ExecuteGetHit(AActor* OtherActor, FVector ImpactPoint)
{
	if (IHitInterface* HitInterface = Cast<IHitInterface>(OtherActor))
	{
		HitInterface->Execute_GetHit(OtherActor, ImpactPoint, this);
	}
}


void ABaseCharacter::PlaySound(const FVector& ImpactPoint, USoundBase* PlayedSound)
{
	if (PlayedSound)
  {
    UGameplayStatics::PlaySoundAtLocation(
      this,
      PlayedSound,
      ImpactPoint
    );
  }
}

void ABaseCharacter::SpawnParticles(const FVector& ImpactPoint, UParticleSystem* SpawnedParticles)
{
	if (SpawnedParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			SpawnedParticles,
			ImpactPoint
		);
	}
}


void ABaseCharacter::CreateHandFootBox(FName BoxName, FName SocketName)
{
	HandFootBoxes.Add(CreateDefaultSubobject<UBoxComponent>(BoxName));
  HandFootBoxes.Last()->SetupAttachment(GetMesh(), SocketName);
  HandFootBoxes.Last()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}



void ABaseCharacter::SetupHandFoot(int32 BodyIndex, float Damage, FName SocketName)
{
	if (BodyIndex < HandsAndFeet.Num() && HandsAndFeet[BodyIndex]) 
	{
		HandsAndFeet[BodyIndex]->Equip(GetMesh(), SocketName, this, this);
		HandsAndFeet[BodyIndex]->SetDamage(Damage);
		HandsAndFeet[BodyIndex]->GetBox()->SetVisibility(true);
		if (HandFootBoxes[BodyIndex])
		{
			HandsAndFeet[BodyIndex]->GetBox()->SetBoxExtent(HandFootBoxes[BodyIndex]->GetUnscaledBoxExtent());
			HandsAndFeet[BodyIndex]->GetBox()->SetWorldTransform(HandFootBoxes[BodyIndex]->GetComponentTransform());
		}
	}
}


int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (!Montage) return -1;
	if (SectionNames.Num() <= 0) return -1;

	int32 MaxSectionIndex = SectionNames.Num() - 1;
	int32 Selection = FMath::RandRange(0, MaxSectionIndex);

	PlayMontageSection(Montage, SectionNames[Selection]);

	return Selection;
}