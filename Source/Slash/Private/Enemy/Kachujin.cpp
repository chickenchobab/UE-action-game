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
  AcceptanceRadius = 200.f;
	GetCharacterMovement()->MaxAcceleration = 10000.f;

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

  SetupHandFoot(0, 1.f, FName("RightHandSocket"));
	SetupHandFoot(1, 1.f, FName("LeftHandSocket"));
  SetupHandFoot(2, 1.f, FName("RightFootSocket"));
	SetupHandFoot(3, 1.f, FName("LeftFootSocket"));
}

void AKachujin::Attack()
{ 
  Super::Attack();
  if (!CombatTarget) return;

  EnemyState = EEnemyState::EES_Engaged;
  FocusOnTarget();
	if (IsTargetInRange(CombatTarget, PunchKickRadius))
	{
    PunchOrKick();
	}
	else
	{
		FireEnergyWave();
	}
}

bool AKachujin::CanAttack()
{
	return Super::CanAttack() && IsTargetInRange(CombatTarget, EnergyWaveRadius);
}

void AKachujin::SpawnProjectile()
{
	if (UWorld *World = GetWorld())
  {
    if (Projectile = World->SpawnActor<ARangedWeapon>(ProjectileClass))
		{
			UE_LOG(LogTemp, Warning, TEXT("Fire ball spawned"));
			Projectile->Equip(GetMesh(), FName("RightHandSocket"), this, this, false, false);
			Projectile->SetHeadDirection(FVector(1, 0, 0));
			Projectile->GetBox()->OnComponentBeginOverlap.AddDynamic(this, &AKachujin::ProjectileHit);
		}
  }
}

void AKachujin::FireProjectile()
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

void AKachujin::CheckCombatTarget()
{
	if (!IsTargetInRange(CombatTarget, CombatRadius))
  {
		ClearAttackTimer();
    LoseInterest();
		ResetCombo();
		if (!IsEngaged())
		{
			StartPatrolling();
		}
  }
	else if (!IsTargetInRange(CombatTarget, EnergyWaveRadius) && !IsChasing())
	{
		ClearAttackTimer();
		ResetCombo();
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
	MoveToTarget(CombatTarget, PunchKickRadius);
}


void AKachujin::RushStart() // Kick motion starts while rushing montage is playing
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
	CheckCombatTarget();
}

void AKachujin::PunchOrKick()
{
	if (bInPunchCombo)
	{
		PunchRight();
	}
	else
	{
		int32 Selection = FMath::RandRange(0, 1);
		Selection == 0 ? PunchLeft() : Kick();
	}
}


void AKachujin::PunchLeft()
{
	PlayRandomMontageSection(LeftPunchMontage, LeftPunchMontageSections);
	bInPunchCombo = true;
	GetWorldTimerManager().ClearTimer(PunchComboWindowTimer);
	GetWorldTimerManager().SetTimer(PunchComboWindowTimer, this, &AKachujin::ResetCombo, PunchComboWindow);
	AttackTime = 0.01f;
}


void AKachujin::PunchRight()
{
	PlayRandomMontageSection(RightPunchMontage, RightPunchMontageSections);
	bInPunchCombo = false;
	AttackTime = 0.5f;
}


void AKachujin::Kick()
{
	PlayRandomMontageSection(KickMontage, KickMontageSections);
}


void AKachujin::FireEnergyWave()
{
	bInPunchCombo = false;
	SpawnProjectile();
	PlayMontage(ThrowMontage);
}