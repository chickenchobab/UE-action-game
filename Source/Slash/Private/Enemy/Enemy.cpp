// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"
#include "AIController.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/AnimMontage.h"
#include "MotionWarpingComponent.h"
#include "DrawDebugHelpers.h"
#include "NavigationSystem.h"

#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Soul.h"
#include "Characters/SlashCharacter.h"
#include "Items/Weapons/RangedWeapon.h"


AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);

	HealthBarComponent = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetupAttachment(GetRootComponent());

	AIPerception = CreateDefaultSubobject<UAIPerceptionComponent>(TEXT("AIPerception"));
	SightConfig = CreateDefaultSubobject<UAISenseConfig_Sight>(TEXT("SightConfig"));

	SightConfig->SightRadius = 4000.f;
	SightConfig->LoseSightRadius = 4200.f;
	SightConfig->PeripheralVisionAngleDegrees = 45.f;
	SightConfig->DetectionByAffiliation.bDetectEnemies = true;
	SightConfig->DetectionByAffiliation.bDetectNeutrals = true;
	SightConfig->DetectionByAffiliation.bDetectFriendlies = true;
	AIPerception->ConfigureSense(*SightConfig);
	AIPerception->SetDominantSense(SightConfig->GetSenseImplementation());

	MotionWarping = CreateDefaultSubobject<UMotionWarpingComponent>(TEXT("MotionWarping"));
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!IsAlive()) return;

	if (CombatTarget)
	{
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget();
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	return DamageAmount;
}

void AEnemy::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}


void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter, bool bReact)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter, bReact);

	ClearPatrolTimer();
	ClearAttackTimer();
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	
	if (IsAlive())
	{
		if (bReact)
		{
			EnemyState = EEnemyState::EES_Engaged; // Hit reacting
		}
		GainInterest(Hitter);
	}
}


void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	EnemyController = Cast<AAIController>(GetController());
	AIPerception->OnPerceptionUpdated.AddDynamic(this, &AEnemy::PerceptionUpdated);

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	HideHealthBar();
	
	MoveToTarget(PatrolTarget = ChoosePatrolTarget()); // Should move first so that a turnabout occurs.

	Tags.Add(FName("Enemy"));
}

void AEnemy::Attack()
{
	Super::Attack();

	if (CombatTarget && CombatTarget->ActorHasTag("Dead"))
	{
		LoseInterest();
		StartPatrolling();
	}
}

void AEnemy::Die()
{
	Super::Die();

	EnemyState = EEnemyState::EES_Dead;
	HideHealthBar();
	SetLifeSpan(DeathLifeSpan);
	SetCapsuleCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->bOrientRotationToMovement = false;
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	SpawnSoul();
}

void AEnemy::Parry()
{
	Super::Parry();
	
	EnemyState = EEnemyState::EES_Parrying;
}

void AEnemy::AttackEnd()
{
	Super::AttackEnd();

	EnemyState = EEnemyState::EES_None; 
	CheckCombatTarget();
}


void AEnemy::HitReactEnd()
{
	Super::HitReactEnd();

	EnemyState = EEnemyState::EES_None;
	CheckCombatTarget();
}


bool AEnemy::CanAttack()
{
	return !IsAttacking() && !IsEngaged();
}

void AEnemy::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
	
	if (Attributes && HealthBarComponent)
	{
		Attributes->ReceiveDamage(DamageAmount);
		HealthBarComponent->SetHealthPercent(Attributes->GetHealthPercent());
	}
}


void AEnemy::RotateProjectile()
{
	if (Projectile == nullptr || CombatTarget == nullptr) return;
	
	FVector ForwardVector = CombatTarget->GetActorLocation() - Projectile->GetActorLocation();
	FVector UpVector = CombatTarget->GetActorUpVector();
	FVector RightVector = ForwardVector.Cross(UpVector);

  FMatrix RotationMatrix(ForwardVector, RightVector, UpVector, FVector::Zero());
  Projectile->SetActorRotation(RotationMatrix.Rotator());
}


FVector AEnemy::GetTranslationWarpTarget()
{
	if (CombatTarget == nullptr) return FVector();

	const FVector CombatTargetLocation = CombatTarget->GetActorLocation();
	const FVector Location = GetActorLocation();

	FVector TargetToMe = (Location - CombatTargetLocation).GetSafeNormal();
	TargetToMe *= WarpTargetDistance;
	
	return CombatTargetLocation + TargetToMe;
}


FVector AEnemy::GetRotationWarpTarget()
{
	if (CombatTarget == nullptr) return FVector();
	return CombatTarget->GetActorLocation();
}


void AEnemy::CheckCombatTarget()
{
  
}

void AEnemy::CheckPatrolTarget()
{
	if (IsTargetInRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();
		float WaitTime = FMath::RandRange(PatrolWaitMin, PatrolWaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

void AEnemy::MoveToTarget(AActor* Target)
{
	if (!EnemyController || !Target) return;

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(AcceptanceRadius);
	EnemyController->MoveTo(MoveRequest);
}


void AEnemy::MoveToTarget(AActor* Target, double Radius)
{
	if (!EnemyController || !Target) return;

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(Radius);
	EnemyController->MoveTo(MoveRequest);
}


APawn* AEnemy::FindCombatTarget(const TArray<AActor*>& UpdatedActors)
{
	for (AActor* UpdatedActor : UpdatedActors)
	{
		if (IsOpposite(UpdatedActor) && !UpdatedActor->ActorHasTag("Dead"))
		{
			return Cast<APawn>(UpdatedActor);
		}
	}
	return nullptr;
}

bool AEnemy::IsTargetInRange(AActor* Target, double Radius)
{
	if (!Target) return false;
	return (Target->GetActorLocation() - GetActorLocation()).Size() <= Radius;
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(PatrolTarget);
}

AActor* AEnemy::ChoosePatrolTarget()
{
	TArray<AActor*> ValidTargets;
	for (AActor* Target : PatrolTargets)
	{
		if (Target == PatrolTarget) continue;
		ValidTargets.AddUnique(Target);
	}

	const int32 NumPatrolTargets = ValidTargets.Num();
	if (NumPatrolTargets > 0)
	{
		const int32 TargetSelection = FMath::RandRange(0, NumPatrolTargets - 1);
		return ValidTargets[TargetSelection];
	}
	return nullptr;
}


void AEnemy::ShowHealthBar()
{
	if (HealthBarComponent)
	{
		HealthBarComponent->SetVisibility(true);
	}
}

void AEnemy::HideHealthBar()
{
	if (HealthBarComponent)
	{
		HealthBarComponent->SetVisibility(false);
	}
}


void AEnemy::GainInterest(AActor* NewTarget)
{
  CombatTarget = NewTarget;
  ShowHealthBar();
}

void AEnemy::LoseInterest()
{
  CombatTarget = nullptr;
  HideHealthBar();
}


void AEnemy::StartPatrolling()
{
	EnemyState = EEnemyState::EES_Patrolling;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemy::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

void AEnemy::StartAttacking(float AttackDelay)
{
	EnemyState = EEnemyState::EES_Attacking;
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackDelay);
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}

void AEnemy::SpawnDefaultWeapon()
{
	if (UWorld* World = GetWorld())
	{
		if (AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass))
		{
			DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
			EquippedWeapon = DefaultWeapon;
		}
	}
}

void AEnemy::PerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (EnemyState != EEnemyState::EES_None && EnemyState != EEnemyState::EES_Patrolling)
	{
		return;
	}

	APawn* SeenPawn = FindCombatTarget(UpdatedActors);
	if (!SeenPawn)
	{
		return;
	}

	// This function is called less frequently than frame.
	ClearPatrolTimer();
	GainInterest(SeenPawn);
	ChaseTarget();
}

void AEnemy::SpawnSoul()
{
	UWorld* World = GetWorld();
	if (World && SoulClass && Attributes)
	{
		// Why the soul spawned below
		ASoul* SpawnedSoul = World->SpawnActor<ASoul>(SoulClass, GetActorLocation(), GetActorRotation());
		// is destroyed before the if statement
		if (SpawnedSoul)
		{
			SpawnedSoul->SetSouls(Attributes->GetSouls());
			SpawnedSoul->AllowOverlapEvent();
		}
	}
}

bool AEnemy::IsTargetAttacking()
{
	if (ASlashCharacter* SlashCharacter = Cast<ASlashCharacter>(CombatTarget))
	{
		return IsTargetInRange(CombatTarget, 100.f) && SlashCharacter->IsAttacking();
	}
	return false;
}

void AEnemy::StopParrying()
{
	StopMontage(0.5f, ParryMontage);
	EnemyState = EEnemyState::EES_None; 
}


void AEnemy::FocusOnTarget()
{
	if (!CombatTarget) return;

	FVector ToTarget = CombatTarget->GetActorLocation() - GetActorLocation();
	ToTarget.Z = 0;
	FRotator LookAtRotation = ToTarget.Rotation();
	SetActorRotation(LookAtRotation);
}


bool AEnemy::DetachFromTarget()
{
	if (!CombatTarget) return false;

	float EscapeDistance = AcceptanceRadius - (GetActorLocation() - CombatTarget->GetActorLocation()).Size();
	FVector EscapeDirection = (GetActorLocation() - CombatTarget->GetActorLocation()).GetSafeNormal();
	FVector EscapeLocation = GetActorLocation() + EscapeDistance * EscapeDirection;

	UNavigationSystemV1* NavSys = UNavigationSystemV1::GetCurrent(GetWorld());
	FNavLocation NavLocation;

	if (EnemyController && NavSys && NavSys->ProjectPointToNavigation(EscapeLocation, NavLocation))
	{
    if (EnemyController->MoveToLocation(NavLocation.Location) == EPathFollowingRequestResult::RequestSuccessful)
		{
			EnemyState = EEnemyState::EES_Detaching;
			GetCharacterMovement()->bOrientRotationToMovement = false;
			GetCharacterMovement()->MaxWalkSpeed = DetachingSpeed;
			FocusOnTarget();
			return true;
		}
	}
	return false;
}


void AEnemy::StopDetaching()
{
	GetCharacterMovement()->bOrientRotationToMovement = true;
	FocusOnTarget();
	EnemyState = EEnemyState::EES_None;
}
