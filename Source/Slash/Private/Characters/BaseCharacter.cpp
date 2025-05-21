// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/BaseCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/CapsuleComponent.h"
#include "Items/Weapons/MeleeWeapon.h"
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
	if (IsParrying())
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
	if (Tags.IsEmpty() || !OtherActor || OtherActor->Tags.IsEmpty()) return false;
  return Tags[0] != OtherActor->Tags[0];
}

void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	for (int i = 0; i < BodyBoxes.Num(); ++i)
	{
		if (BodyBoxes[i])
		{
			BodyBoxes[i]->OnComponentBeginOverlap.AddDynamic(this, &ABaseCharacter::BodyBoxOverlap);
		}
	}
}

void ABaseCharacter::Attack()
{
	
}

void ABaseCharacter::Die()
{
	Tags.Add("Dead");
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
	
	SetBodyBlocked(false);
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

void ABaseCharacter::BodyBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (!IsBodyBlocked() && IsOpposite(OtherActor))
	{
		UGameplayStatics::ApplyDamage(OtherActor, BodyAttackDamage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
		ExecuteGetHit(SweepResult);
		UE_LOG(LogTemp, Warning, TEXT("OtherActor(%s) and SweepResultActor(%s)"), *OtherActor->GetName(), *SweepResult.GetActor()->GetName());
	}
}


void ABaseCharacter::SetCapsuleCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	GetCapsuleComponent()->SetCollisionEnabled(CollisionEnabled);
}

void ABaseCharacter::SetWeaponCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->SetWeaponBoxCollisionEnabled(CollisionEnabled);
		EquippedWeapon->ResetActorsToIgnore();
	}
}


void ABaseCharacter::SetBodyCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
	for (int i = 0; i < BodyBoxes.Num(); ++i)
	{
		if (BodyBoxes[i])
		{
			BodyBoxes[i]->SetCollisionEnabled(CollisionEnabled);
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

void ABaseCharacter::PlayDodgeMontage()
{
	PlayMontageSection(DodgeMontage, FName("Default"));
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



void ABaseCharacter::ExecuteGetHit(const FHitResult &HitResult)
{
	if (IHitInterface* HitInterface = Cast<IHitInterface>(HitResult.GetActor()))
	{
		UE_LOG(LogTemp, Warning, TEXT("BaseCharacter ExecuteHit : %s"), *HitResult.GetActor()->GetName());
		HitInterface->Execute_GetHit(HitResult.GetActor(), HitResult.ImpactPoint, this);
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

int32 ABaseCharacter::PlayRandomMontageSection(UAnimMontage* Montage, const TArray<FName>& SectionNames)
{
	if (!Montage) return -1;
	if (SectionNames.Num() <= 0) return -1;

	int32 MaxSectionIndex = SectionNames.Num() - 1;
	int32 Selection = FMath::RandRange(0, MaxSectionIndex);

	PlayMontageSection(Montage, SectionNames[Selection]);

	return Selection;
}