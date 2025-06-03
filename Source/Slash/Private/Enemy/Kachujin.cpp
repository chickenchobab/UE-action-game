// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Kachujin.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Weapons/RangedWeapon.h"
#include "MotionWarpingComponent.h"
#include "NiagaraComponent.h"
#include "Components/BoxComponent.h"
#include "AIController.h"
#include "GameFramework/CharacterMovementComponent.h"

AKachujin::AKachujin()
{
	CombatRadius = 1000.f;
  AcceptanceRadius = 100.f;
  AttackRadius = 100.f; // Basically punches and kicks away sometimes
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
			UE_LOG(LogTemp, Warning, TEXT("Fire ball spawned"));
			Throwing->Equip(GetMesh(), FName("RightHandSocket"), this, this, false, false);
			Throwing->SetHeadDirection(FVector(1, 0, 0));
			Throwing->GetBox()->OnComponentBeginOverlap.AddDynamic(this, &AKachujin::ProjectileHit);
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


void AKachujin::ProjectileHit(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
	if (OtherActor == nullptr || OtherActor == this) return;

	if (OtherActor == CombatTarget)
	{
		UE_LOG(LogTemp, Warning, TEXT("Rush start"));
		RushToTarget();
	}
}


void AKachujin::RushToTarget()
{
	EnemyState = EEnemyState::EES_Engaged;
	PlayMontage(RushMontage);
	EnemyController->ReceiveMoveCompleted.RemoveDynamic(this, &AKachujin::MoveCompleted);
	EnemyController->ReceiveMoveCompleted.AddDynamic(this, &AKachujin::MoveCompleted);
	bMoveCompleted = false;
	GetCharacterMovement()->MaxWalkSpeed = RushingSpeed;
	MoveToTarget(CombatTarget);
}


void AKachujin::RushStart() // Kick motion starts
{
	// Move can be completed earlier than kick motion
	if (bMoveCompleted) return;

	PauseMontage(RushMontage);
}


void AKachujin::MoveCompleted(FAIRequestID RequestID, EPathFollowingResult::Type Result)
{
	if (Result == EPathFollowingResult::Success)
	{
		UE_LOG(LogTemp, Warning, TEXT("Move completed"));
	}
	else 
	{
		UE_LOG(LogTemp, Warning, TEXT("Move Failed"));
	}

	ResumeMontage(RushMontage);
	bMoveCompleted = true;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	// EnemyController->ReceiveMoveCompleted.RemoveDynamic(this, &AKachujin::MoveCompleted);
}