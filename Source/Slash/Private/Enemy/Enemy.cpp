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
// #include "DrawDebugHelpers.h"

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

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	HandleDamage(DamageAmount);
	GainInterest(EventInstigator->GetPawn());
	ChaseTarget();
	return DamageAmount;
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

void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	EnemyController = Cast<AAIController>(GetController());
	AIPerception->OnPerceptionUpdated.AddDynamic(this, &AEnemy::PerceptionUpdated);

	HideHealthBar();
	MoveToTarget(PatrolTarget = ChoosePatrolTarget()); // Should move first so that a turnabout occurs.
	SpawnDefaultWeapon();

	Tags.Add(FName("Enemy"));
}

void AEnemy::Attack()
{
	EnemyState = EEnemyState::EES_Engaged;
	PlayAttackMontage();
}

 void AEnemy::Die()
{
	EnemyState = EEnemyState::EES_Dead;
	PlayDeathMontage();
	SetLifeSpan(DeathLifeSpan);
	HideHealthBar();
	SetCapsuleCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCharacterMovement()->bOrientRotationToMovement = false;
}

void AEnemy::AttackEnd()
{
	EnemyState = EEnemyState::EES_None;
	CheckCombatTarget();
}

bool AEnemy::CanAttack()
{
	return !IsAttacking() && !IsEngaged() && IsTargetInRange(CombatTarget, AttackRadius);
}

void AEnemy::HandleDamage(float DamageAmount)
{
	if (Attributes && HealthBarComponent)
	{
		Attributes->ReceiveDamage(DamageAmount);
		HealthBarComponent->SetHealthPercent(Attributes->GetHealthPercent());
	}
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


void AEnemy::CheckCombatTarget()
{
  if (!IsTargetInRange(CombatTarget, CombatRadius))
  {
		ClearAttackTimer();
    LoseInterest();
		if (!IsEngaged())
		{
			StartPatrolling();
		}
		UE_LOG(LogTemp, Warning, TEXT("Lose Interest"));
  }
	else if (!IsTargetInRange(CombatTarget, AttackRadius) && !IsChasing())
	{
		ClearAttackTimer();
		if (!IsEngaged())
		{
			ChaseTarget();
		}
		UE_LOG(LogTemp, Warning, TEXT("Chase Player"));
	}
	else if (CanAttack())
	{
		StartAttacking();	
		UE_LOG(LogTemp, Warning, TEXT("Attack Player"));
	}
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
	MoveRequest.SetAcceptanceRadius(50.f);
	EnemyController->MoveTo(MoveRequest);
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
	GetCharacterMovement()->MaxWalkSpeed = PatrollingSpeed;
	MoveToTarget(PatrolTarget);
}

void AEnemy::ChaseTarget()
{
	EnemyState = EEnemyState::EES_Chasing;
	GetCharacterMovement()->MaxWalkSpeed = ChasingSpeed;
	MoveToTarget(CombatTarget);
}

void AEnemy::StartAttacking()
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

void AEnemy::SpawnDefaultWeapon()
{
if (UWorld* World = GetWorld())
	{
		AWeapon* DefaultWeapon = World->SpawnActor<AWeapon>(WeaponClass);
		DefaultWeapon->Equip(GetMesh(), FName("RightHandSocket"), this, this);
		EquippedWeapon = DefaultWeapon;
	}
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

