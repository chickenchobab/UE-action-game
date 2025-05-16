// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectile::AProjectile()
{
  ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
  ProjectileMovement->bAutoActivate = false;
  ProjectileMovement->bInitialVelocityInLocalSpace = true;
  ProjectileMovement->ProjectileGravityScale = 0.1f;
  ProjectileMovement->InitialSpeed = 2000.f;
  ProjectileMovement->MaxSpeed = 2500.f;
  // ProjectileMovement->Deactivate();
  
  TrailParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Trail Particles"));
  TrailParticles->SetupAttachment(GetRootComponent());
}


void AProjectile::ActivateProjectile(AActor* CombatTarget)
{
  RotateTowardsTarget(CombatTarget);
  if (ProjectileMovement)
  {
    ProjectileMovement->SetVelocityInLocalSpace(FVector(0, 0, -1) * ProjectileMovement->InitialSpeed);
    ProjectileMovement->Activate();
  }
}


void AProjectile::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
  if (IsBlocked())
  {
    UE_LOG(LogTemp, Warning, TEXT("Blocked"));
    return;
  }
  if (!IsOwnerOpposite(OtherActor))
  {
    UE_LOG(LogTemp, Warning, TEXT("Not opposite"));
    return;
  }

  UE_LOG(LogTemp, Warning, TEXT("Projectile overlaps"));
  UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
  ExecuteGetHit(SweepResult);
  Destroy();
}

void AProjectile::RotateTowardsTarget(AActor* CombatTarget)
{
  if (CombatTarget == nullptr) return;

  // Mesh-specific calculation
  FVector RightVector = CombatTarget->GetActorUpVector();
  FVector UpVector = -(CombatTarget->GetActorLocation() - GetActorLocation());
  FVector ForwardVector = UpVector.Cross(RightVector);

  FMatrix RotationMatrix(ForwardVector, RightVector, UpVector, FVector::Zero());
  SetActorRotation(RotationMatrix.Rotator());
}