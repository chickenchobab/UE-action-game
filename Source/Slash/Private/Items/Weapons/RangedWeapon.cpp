// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/RangedWeapon.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

ARangedWeapon::ARangedWeapon()
{
  ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
  ProjectileMovement->bAutoActivate = false;
  ProjectileMovement->bInitialVelocityInLocalSpace = true;
  ProjectileMovement->ProjectileGravityScale = 0.1f;
  ProjectileMovement->InitialSpeed = 2000.f;
  ProjectileMovement->MaxSpeed = 2500.f;
  
  TrailParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Trail Particles"));
  TrailParticles->SetupAttachment(GetRootComponent());
}


void ARangedWeapon::ActivateProjectile(AActor* CombatTarget)
{
  if (ProjectileMovement)
  {
    ProjectileMovement->SetVelocityInLocalSpace(FVector(0, 0, -1) * ProjectileMovement->InitialSpeed);
    ProjectileMovement->Activate();
  }
}


void ARangedWeapon::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{ 
  if (OtherActor)
  {
    UE_LOG(LogTemp, Warning, TEXT("Hit actor : %s"), *OtherActor->GetName());
  }

  if (GetOwner() && GetOwner() == OtherActor) return;

  if (!IsBlocked() && IsOwnerOpposite(OtherActor))
  {
    UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
    ExecuteGetHit(OtherActor, SweepResult.ImpactPoint);
  }
  Destroy();
}

void ARangedWeapon::RotateTowardsTarget(AActor* CombatTarget)
{
  if (CombatTarget == nullptr) return;

  // Mesh-specific calculation
  FVector RightVector = CombatTarget->GetActorUpVector();
  FVector UpVector = -(CombatTarget->GetActorLocation() - GetActorLocation());
  FVector ForwardVector = UpVector.Cross(RightVector);

  FMatrix RotationMatrix(ForwardVector, RightVector, UpVector, FVector::Zero());
  SetActorRotation(RotationMatrix.Rotator());
}