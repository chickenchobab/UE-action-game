// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Sight.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/AnimMontage.h"
#include "AIController.h"

#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"
#include "Items/Weapons/Weapon.h"


AEnemy::AEnemy()
{
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

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

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	HideHealthBar();
	EnemyController = Cast<AAIController>(GetController());
	// Should move first so that a turnabout occurs.
	MoveToTarget(CurrentPatrolTarget = ChoosePatrolTarget());

	AIPerception->OnPerceptionUpdated.AddDynamic(this, &AEnemy::PerceptionUpdated);

	if (UWorld* World = GetWorld())
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
}

void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (EnemyState == EEnemyState::EES_Dead) return;

	if (CombatTarget)
	{
		CheckCombatTarget();
	}
	else
	{
		CheckPatrolTarget();
	}
}


void AEnemy::Destroyed()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->Destroy();
	}
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, const FVector& HitterLocation)
{
	PlayHitSound(ImpactPoint);
	SpawnHitParticles(ImpactPoint);

	if (Attributes && Attributes->IsAlive())
	{
		DirectionalHitReact(ImpactPoint, HitterLocation);
	}
	else
	{
		Die();
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	GainInterest(EventInstigator->GetPawn());
	ChaseTarget();
	return DamageAmount;
}


void AEnemy::Attack()
{
	Super::Attack();

	EnemyState = EEnemyState::EES_Engaged;
	PlayAttackMontage();
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_None;
	CheckCombatTarget();
}


int32 AEnemy::PlayDeathMontage()
{
	int32 Selection = Super::PlayDeathMontage();
	if (Selection < 0) return -1;
	
	EDeathPose Pose = static_cast<EDeathPose>(Selection);
	if (Pose >= EDeathPose::EDP_MAX) return -1;

	DeathPose = Pose;
	return Selection;
}

APawn* AEnemy::FindPlayer(const TArray<AActor*>& UpdatedActors)
{
	for (AActor* UpdatedActor : UpdatedActors)
	{
		UE_LOG(LogTemp, Warning, TEXT("Updated Actor : %s"), *UpdatedActor->GetActorNameOrLabel());
		if (UpdatedActor->ActorHasTag(FName("SlashCharacter")))
		{
			return Cast<APawn>(UpdatedActor);
		}
	}
	return nullptr;
}

void AEnemy::PerceptionUpdated(const TArray<AActor*>& UpdatedActors)
{
	if (EnemyState == EEnemyState::EES_Chasing) return;

	APawn* SeenPawn = FindPlayer(UpdatedActors);
	if (!SeenPawn) return;

	UE_LOG(LogTemp, Warning, TEXT("Found pawn"));

	GetWorldTimerManager().ClearTimer(PatrolTimer);
	GainInterest(SeenPawn);
}


void AEnemy::Die()
{
	EnemyState = EEnemyState::EES_Dead;
	PlayDeathMontage();
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(DeathLifeSpan);
	HideHealthBar();
	GetCharacterMovement()->bOrientRotationToMovement = false;
}


bool AEnemy::IsTargetInRange(AActor* Target, double Radius)
{
	if (!Target) return false;
	return (Target->GetActorLocation() - GetActorLocation()).Size() <= Radius;
}

void AEnemy::CheckCombatTarget()
{
  if (!IsTargetInRange(CombatTarget, CombatRadius))
  {
		ClearAttackTimer();
    LoseInterest();
		// if not engaged
    StartPatrolling();
		UE_LOG(LogTemp, Warning, TEXT("Lose Interest"));
  }
	else if (!IsTargetInRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Chasing)
	{
		ClearAttackTimer();
		// if not engaged
		ChaseTarget();
		UE_LOG(LogTemp, Warning, TEXT("Chase Player"));
	}
	else if (IsTargetInRange(CombatTarget, AttackRadius) && EnemyState != EEnemyState::EES_Attacking)
	{
		StartAttackTimer();
		UE_LOG(LogTemp, Warning, TEXT("Attack Player"));
	}
}

void AEnemy::CheckPatrolTarget()
{
	if (IsTargetInRange(CurrentPatrolTarget, PatrolRadius))
	{
		CurrentPatrolTarget = ChoosePatrolTarget();
		float WaitTime = FMath::RandRange(WaitMin, WaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, WaitTime);
	}
}

void AEnemy::PatrolTimerFinished()
{
	MoveToTarget(CurrentPatrolTarget);
}

AActor* AEnemy::ChoosePatrolTarget()
{
	TArray<AActor*> ValidTargets;
	for (AActor* Target : PatrolTargets)
	{
		if (Target == CurrentPatrolTarget) continue;
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

void AEnemy::MoveToTarget(AActor* Target)
{
	if (!EnemyController || !Target) return;

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(50.f);
	EnemyController->MoveTo(MoveRequest);
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
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(CurrentPatrolTarget);
}

void AEnemy::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

void AEnemy::HandleDamage(float DamageAmount)
{
if (Attributes && HealthBarComponent)
	{
		Attributes->ReceiveDamage(DamageAmount);
		HealthBarComponent->SetHealthPercent(Attributes->GetHealthPercent());
	}
}


void AEnemy::StartAttackTimer()
{
	EnemyState = EEnemyState::EES_Attacking;
	GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemy::Attack, AttackTime);
}

void AEnemy::ClearPatrolTimer()
{
	GetWorldTimerManager().ClearTimer(PatrolTimer);
}

void AEnemy::ClearAttackTimer()
{
	GetWorldTimerManager().ClearTimer(AttackTimer);
}