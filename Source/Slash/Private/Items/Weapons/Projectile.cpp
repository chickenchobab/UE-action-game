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
  ProjectileMovement->InitialSpeed = 1400.f;
  ProjectileMovement->MaxSpeed = 2000.f;
  // ProjectileMovement->Deactivate();
  
  TrailParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Trail Particles"));
  TrailParticles->SetupAttachment(GetRootComponent());
}


void AProjectile::ActivateProjectile()
{
  if (ProjectileMovement)
  {
    UE_LOG(LogTemp, Warning, TEXT("Activated! : %f"), ProjectileMovement->InitialSpeed);
    // if (GetOwner())
    // {
    //   FRotator OwnerRotation = GetOwner()->GetActorRotation();
    //   SetActorRotation(GetOwner()->GetActorRotation());
    // }
    ProjectileMovement->SetVelocityInLocalSpace(FVector(0, 0, -1) * ProjectileMovement->InitialSpeed);
    ProjectileMovement->Activate();
  }
}


void AProjectile::BeginPlay()
{
  Super::BeginPlay();
}


void AProjectile::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
  // if (IsBlocked() || !IsOwnerOpposite(OtherActor)) return;

  // UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
  // ExecuteGetHit(SweepResult);
}

