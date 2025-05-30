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
  AttackRadius = 200.f; // Use dual dagger
  SpecialAttackRadius = 700.f; // Throw shuriken
	AttackTime = 0.5f;

	CreateHandFootBox(FName("Right Foot Box"), FName("RightFootSocket"));
	CreateHandFootBox(FName("Left Foot Box"), FName("LeftFootSocket"));
}

void ANinja::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
  
  if (MotionWarping)
	{
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
	if (IsTargetInRange(CombatTarget, AttackRadius))
	{
		PlayAttackMontage();
	}
	else
	{
		SpawnProjectile();
		PlaySpecialAttackMontage();
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
	if (IsTargetInRange(CombatTarget, AttackRadius))
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
	return Super::CanAttack() && IsTargetInRange(CombatTarget, SpecialAttackRadius);
}

void ANinja::SpawnProjectile()
{
	if (UWorld *World = GetWorld())
  {
    if (Throwing = World->SpawnActor<ARangedWeapon>(ThrowingClass))
		{
			Throwing->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		}
  }
}

void ANinja::FireProjectile()
{
	if (Throwing)
  {
    Throwing->DetachMeshFromSocket();
    if (CombatTarget)
    {
      Throwing->ActivateProjectile(CombatTarget);
    }
    SetWeaponCollisionEnabled(ECollisionEnabled::QueryOnly);
    Throwing = nullptr;
  }
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
	else if (!IsTargetInRange(CombatTarget, SpecialAttackRadius) && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget();
		}
	}
	else if (IsTargetInRange(CombatTarget, AcceptanceRadius) && !IsTargetInRange(CombatTarget, AttackRadius) && !IsDetaching())
  {
    if (!IsEngaged() && DetachFromTarget()) // Can step back
    {
      ClearAttackTimer();
      UE_LOG(LogTemp, Warning, TEXT("Detaching"));
    }
    else if (CanAttack()) // Cannot but attack
    {
      StartAttacking(AttackTime);
      UE_LOG(LogTemp, Warning, TEXT("Detaching Attack"));
    }
  }
	else if (IsTargetAttacking() && !IsEngaged())
	{
		ClearAttackTimer();
		FocusOnTarget();
		Dodge();
		UE_LOG(LogTemp, Warning, TEXT("Dodge"));
	}
	else if (CanAttack())
	{	
		StartAttacking(AttackTime);
		UE_LOG(LogTemp, Warning, TEXT("Attack"));
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

void ANinja::BackAttack()
{
	EnemyState = EEnemyState::EES_Engaged;
	FocusOnTarget();
	ReverseWeaponMesh();
	PlayBackAttackMontage();
}

void ANinja::PlayBackAttackMontage()
{
	PlayMontageSection(BackAttackMontage, FName("Default"));
}


void ANinja::KickStart()
{
	RestoreWeaponMesh();
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
