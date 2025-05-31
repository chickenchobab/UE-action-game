// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Kachujin.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Weapons/RangedWeapon.h"
#include "MotionWarpingComponent.h"

AKachujin::AKachujin()
{
	CombatRadius = 1000.f;
  AcceptanceRadius = 400.f;
  AttackRadius = 200.f; // Basically punches and kicks away sometimes
  SpecialAttackRadius = 700.f; // Fires an energy ball and rushes to the target if succeeded

  CreateHandFootBox(FName("Right Hand Box"), FName("RightHandSocket"));
	CreateHandFootBox(FName("Left Hand Box"), FName("LeftHandSocket"));
  CreateHandFootBox(FName("Right Foot Box"), FName("RightFootSocket"));
	CreateHandFootBox(FName("Left Foot Box"), FName("LeftFootSocket"));
}

void AKachujin::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

  if (MotionWarping)
  {
    MotionWarping->AddOrUpdateWarpTargetFromLocation(FName("LocationTarget"), GetTranslationWarpTarget());
    MotionWarping->AddOrUpdateWarpTargetFromLocation(FName("RotationTarget"), GetRotationWarpTarget());
  }
}

void AKachujin::BeginPlay()
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

    {
			AWeapon* BodyWeapon = Cast<AWeapon>(World->SpawnActor(AWeapon::StaticClass()));
			HandsAndFeet.Add(BodyWeapon);
		}
		{
			AWeapon* BodyWeapon = Cast<AWeapon>(World->SpawnActor(AWeapon::StaticClass()));
			HandsAndFeet.Add(BodyWeapon);
			if (HandsAndFeet[2])
			{
				HandsAndFeet[2]->SetPair(BodyWeapon);
			}
		}
  }

  SetupHandFoot(0, 100.f, FName("RightHandSocket"));
	SetupHandFoot(1, 100.f, FName("LeftHandSocket"));
  SetupHandFoot(2, 100.f, FName("RightFootSocket"));
	SetupHandFoot(3, 100.f, FName("LeftFootSocket"));
}

void AKachujin::Attack()
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
}

bool AKachujin::CanAttack()
{
	return Super::CanAttack() && IsTargetInRange(CombatTarget, SpecialAttackRadius);
}

void AKachujin::SpawnProjectile()
{
	if (UWorld *World = GetWorld())
  {
    if (Throwing = World->SpawnActor<ARangedWeapon>(ThrowingClass))
		{
			Throwing->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		}
  }
}

void AKachujin::FireProjectile()
{
	if (Throwing)
  {
    Throwing->DetachMeshFromSocket();
    if (CombatTarget)
    {
			RotateProjectile(Throwing);
      Throwing->ActivateProjectile(CombatTarget);
    }
		Throwing->SetWeaponBoxCollisionEnabled(ECollisionEnabled::QueryOnly);
    Throwing = nullptr;
  }
}

void AKachujin::CheckCombatTarget()
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
	else if (CanAttack())
	{	
		StartAttacking(AttackTime);
		UE_LOG(LogTemp, Warning, TEXT("Attack"));
	}
}

