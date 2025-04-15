// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy/Enemy.h"

#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "Perception/AIPerceptionComponent.h"

#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Animation/AnimMontage.h"
#include "AIController.h"

#include "Components/AttributeComponent.h"
#include "HUD/HealthBarComponent.h"

// Sets default values
AEnemy::AEnemy()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetMesh()->SetGenerateOverlapEvents(true);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);

	Attributes = CreateDefaultSubobject<UAttributeComponent>(TEXT("Attributes"));
	HealthBarComponent = CreateDefaultSubobject<UHealthBarComponent>(TEXT("HealthBar"));
	HealthBarComponent->SetupAttachment(GetRootComponent());

	GetCharacterMovement()->bOrientRotationToMovement = true;
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
}

// Called when the game starts or when spawned
void AEnemy::BeginPlay()
{
	Super::BeginPlay();
	
	if (HealthBarComponent)
	{
		HealthBarComponent->SetHealthPercent(1.f);
		HealthBarComponent->SetVisibility(false);
	}

	EnemyController = Cast<AAIController>(GetController());
	// Should move first so that a turnabout occurs.
	MoveToTarget(PatrolTarget = ChoosePatrolTarget());
}

// Called every frame
void AEnemy::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	CheckCombatTarget();
	CheckPatrolTarget();
}

// Called to bind functionality to input
void AEnemy::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

void AEnemy::GetHit_Implementation(const FVector& ImpactPoint, const FVector& HitterLocation)
{
	if (HealthBarComponent)
	{
		HealthBarComponent->SetVisibility(true);
	}

	if (Attributes && Attributes->IsAlive())
	{
		DirectionalHitReact(ImpactPoint, HitterLocation);
	}
	else
	{
		Die();
	}

	if (HitSound)
  {
    UGameplayStatics::PlaySoundAtLocation(
      this,
      HitSound,
      ImpactPoint
    );
  }

	if (HitParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(
			GetWorld(),
			HitParticles,
			ImpactPoint
		);
	}
}

float AEnemy::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Attributes && HealthBarComponent)
	{
		Attributes->ReceiveDamage(DamageAmount);
		HealthBarComponent->SetHealthPercent(Attributes->GetHealthPercent());
	}

	CombatTarget = EventInstigator->GetPawn();

	return DamageAmount;
}

void AEnemy::PawnSeen(APawn* SeenPawn)
{
	UE_LOG(LogTemp, Warning, TEXT("Found"));
}


void AEnemy::DirectionalHitReact(const FVector& ImpactPoint, const FVector& HitterLocation)
{
	const FVector Forward = GetActorForwardVector();
	// const FVector ImpactPointParallel(ImpactPoint.X, ImpactPoint.Y, GetActorLocation().Z);
	const FVector HitterLocationParallel(HitterLocation.X, HitterLocation.Y, GetActorLocation().Z);
	// const FVector ToHit = (ImpactPointParallel - GetActorLocation()).GetSafeNormal();
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

	// if (GEngine)
	// {
	// 	GEngine->AddOnScreenDebugMessage(1, 5.f, FColor::Black, FString::Printf(TEXT("Theta : %f"), Theta));
	// }
	// UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + Forward * 60.f, 5.f, FColor::Purple, 5.f);
	// UKismetSystemLibrary::DrawDebugArrow(this, GetActorLocation(), GetActorLocation() + ToHit * 60.f, 5.f, FColor::Orange, 5.f);

	if (HitReactMontage)
	{
		PlayHitReactMontage(Section);
	}
}

void AEnemy::PlayHitReactMontage(const FName& SectionName)
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && HitReactMontage)
	{
		AnimInstance->Montage_Play(HitReactMontage);
		AnimInstance->Montage_JumpToSection(SectionName, HitReactMontage);
	}
}

void AEnemy::Die()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && DeathMontage)
	{
		AnimInstance->Montage_Play(DeathMontage);

		const int32 Selection = FMath::RandRange(1, 6);
		FName SectionName = FName("Death" + FString::FromInt(Selection));
		DeathPose = static_cast<EDeathPose>(static_cast<uint8>(EDeathPose::EDP_Alive) + Selection);
		
		AnimInstance->Montage_JumpToSection(SectionName, DeathMontage);
	}

	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	SetLifeSpan(3.f);
	if (HealthBarComponent)
	{
		HealthBarComponent->SetVisibility(false);
	}
}


bool AEnemy::InTargetRange(AActor* Target, double Radius)
{
	if (!Target) return false;
	return (Target->GetActorLocation() - GetActorLocation()).Size() <= Radius;
}

void AEnemy::CheckCombatTarget()
{
  if (!InTargetRange(CombatTarget, CombatRadius))
  {
    CombatTarget = nullptr;
    if (HealthBarComponent)
    {
      HealthBarComponent->SetVisibility(false);
    }
  }
}

void AEnemy::CheckPatrolTarget()
{
	if (InTargetRange(PatrolTarget, PatrolRadius))
	{
		PatrolTarget = ChoosePatrolTarget();
		float Delay = FMath::RandRange(WaitMin, WaitMax);
		GetWorldTimerManager().SetTimer(PatrolTimer, this, &AEnemy::PatrolTimerFinished, Delay);
	}
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

void AEnemy::MoveToTarget(AActor* Target)
{
	if (!EnemyController || !Target) return;

	UE_LOG(LogTemp, Warning, TEXT("Moving"));

	FAIMoveRequest MoveRequest;
	MoveRequest.SetGoalActor(Target);
	MoveRequest.SetAcceptanceRadius(15.f);
	EnemyController->MoveTo(MoveRequest);
}