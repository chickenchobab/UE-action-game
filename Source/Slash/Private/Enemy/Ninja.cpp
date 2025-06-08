// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Ninja.h"
#include "MotionWarpingComponent.h"
#include "Components/CapsuleComponent.h"

#include "Items/Weapons/Weapon.h"
#include "Items/Weapons/MeleeWeapon.h"
#include "Items/Weapons/RangedWeapon.h"

ANinja::ANinja()
{
	CombatRadius = 1000.f;
  AcceptanceRadius = 400.f;
	AttackTime = 0.5f;

	CreateHandFootBox(FName("Right Foot Box"), FName("RightFootSocket"));
	CreateHandFootBox(FName("Left Foot Box"), FName("LeftFootSocket"));
}

void ANinja::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
  
  if (MotionWarping)
	{
		MotionWarping->AddOrUpdateWarpTargetFromLocation(FName("LocationTarget"), GetTranslationWarpTarget());
		MotionWarping->AddOrUpdateWarpTargetFromLocation(FName("RotationTarget"), GetRotationWarpTarget());
	}
}

void ANinja::BeginPlay()
{
	Super::BeginPlay();

  if (UWorld* World = GetWorld())
  {
		{
			AWeapon* BodyWeapon = Cast<AWeapon>(World->SpawnActor(AWeapon::StaticClass()));
			HandsAndFeet.Add(BodyWeapon);
		}
		{
			AWeapon* BodyWeapon = Cast<AWeapon>(World->SpawnActor(AWeapon::StaticClass()));
			HandsAndFeet.Add(BodyWeapon);
			if (HandsAndFeet[0])
			{
				HandsAndFeet[0]->SetPair(BodyWeapon);
			}
		}
  }
  SetupHandFoot(0, 100.f, FName("RightFootSocket"));
	SetupHandFoot(1, 100.f, FName("LeftFootSocket"));

	SpawnDefaultWeapon();
}

void ANinja::Attack()
{
	Super::Attack();

	if (!CombatTarget) return;

	EnemyState = EEnemyState::EES_Engaged;
  FocusOnTarget();
	if (IsTargetInRange(CombatTarget, DaggerRadius))
	{
		DaggerAttack();
	}
	else
	{
		ThrowShuriken();
	}
	AttackTime = 0.5f;
}


void ANinja::Dodge()
{
	Super::Dodge();

	EnemyState = EEnemyState::EES_Engaged;
	DisableCollision();
	PlayDodgeMontage();
}


void ANinja::DodgeEnd()
{
	Super::DodgeEnd();
	
	EnemyState = EEnemyState::EES_None;
	EnableCollision();
	if (IsTargetInRange(CombatTarget, DaggerRadius))
	{
		BackAttack();
	}
	else
	{
		AttackTime = 0.01f;
		CheckCombatTarget();
	}
}


bool ANinja::CanAttack()
{
	return Super::CanAttack() && IsTargetInRange(CombatTarget, ShurikenRadius);
}

void ANinja::SpawnProjectile()
{
	if (UWorld *World = GetWorld())
  {
    if (Projectile = World->SpawnActor<ARangedWeapon>(ProjectileClass))
		{
			Projectile->Equip(GetMesh(), FName("RightHandSocket"), this, this);
			Projectile->SetHeadDirection(FVector(0, 0, 1));
		}
  }
}

void ANinja::FireProjectile()
{
	if (Projectile)
  {
    Projectile->DetachMeshFromSocket();
    if (CombatTarget)
    {
			RotateProjectile();
      Projectile->ActivateProjectile(CombatTarget);
    }
		Projectile->SetWeaponBoxCollisionEnabled(ECollisionEnabled::QueryOnly);
    Projectile = nullptr;
  }
}


void ANinja::RotateProjectile()
{
	if (Projectile == nullptr || CombatTarget == nullptr) return;

	FVector RightVector = CombatTarget->GetActorUpVector();
	FVector UpVector = CombatTarget->GetActorLocation() - Projectile->GetActorLocation();
	FVector ForwardVector = UpVector.Cross(RightVector);

  FMatrix RotationMatrix(ForwardVector, RightVector, UpVector, FVector::Zero());
  Projectile->SetActorRotation(RotationMatrix.Rotator());
}


void ANinja::CheckCombatTarget()
{
	if (!IsTargetInRange(CombatTarget, CombatRadius))
  {
		ClearAttackTimer();
    LoseInterest();
		if (!IsEngaged())
		{
			StartPatrolling();
		}
  }
	else if (!IsTargetInRange(CombatTarget, ShurikenRadius) && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget();
		}
	}
	else if (IsTargetInRange(CombatTarget, AcceptanceRadius) && !IsTargetInRange(CombatTarget, DaggerRadius) && !IsDetaching())
  {
    if (!IsEngaged() && DetachFromTarget()) // Can step back
    {
      ClearAttackTimer();
    }
    else if (CanAttack()) // Cannot but attack
    {
      StartAttacking(AttackTime);
    }
  }
	else if (IsTargetAttacking() && !IsEngaged())
	{
		ClearAttackTimer();
		FocusOnTarget();
		Dodge();
	}
	else if (CanAttack())
	{	
		StartAttacking(AttackTime);
	}
}


void ANinja::SpawnDefaultWeapon()
{
	Super::SpawnDefaultWeapon();

	if (UWorld* World = GetWorld())
	{
		if (AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass))
		{
			DefaultWeapon->Equip(GetMesh(), FName("LeftHandSocket"), this, this);
			if (EquippedWeapon)
			{
				EquippedWeapon->SetPair(DefaultWeapon);
			}
		}
	}
}

void ANinja::DaggerAttack()
{
	PlayMontage(DaggerAttackMontage);
}

void ANinja::ThrowShuriken()
{
	SpawnProjectile();
	PlayRandomMontageSection(ThrowMontage, ThrowMontageSections);
}

void ANinja::BackAttack()
{
	EnemyState = EEnemyState::EES_Engaged;
	FocusOnTarget();
	ReverseWeaponMesh();
	EnableWeaponHitReaction(false);
	PlayMontage(BackAttackMontage);
}

void ANinja::KickStart()
{
	RestoreWeaponMesh();
	EnableWeaponHitReaction(true);
}

void ANinja::EnableCollision()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Block);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Block);
}

void ANinja::DisableCollision()
{
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
}


void ANinja::EnableWeaponHitReaction(bool bEnabled)
{
	if (EquippedWeapon)
	{
		EquippedWeapon->EnableHitReaction(bEnabled);
		if (AWeapon* PairWeapon = EquippedWeapon->GetPair())
		{
			PairWeapon->EnableHitReaction(bEnabled);
		}
	}
}


void ANinja::ReverseWeaponMesh()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->DetachMeshFromSocket();
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocketReversed"));
		if (AWeapon* PairWeapon = EquippedWeapon->GetPair())
		{
			PairWeapon->DetachMeshFromSocket();
			PairWeapon->AttachMeshToSocket(GetMesh(), FName("LeftHandSocketReversed"));
		}
	}
}

void ANinja::RestoreWeaponMesh()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->DetachMeshFromSocket();
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
		if (AWeapon* PairWeapon = EquippedWeapon->GetPair())
		{
			PairWeapon->DetachMeshFromSocket();
			PairWeapon->AttachMeshToSocket(GetMesh(), FName("LeftHandSocket"));
		}
	}
}
