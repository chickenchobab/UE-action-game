// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Particles/ParticleSystemComponent.h"
#include "Kismet/GameplayStatics.h"

AProjectile::AProjectile()
{
  ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
  ProjectileMovement->InitialSpeed = 1400.f;
  ProjectileMovement->MaxSpeed = 2000.f;

  TrailParticles = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("Trail Particles"));
  TrailParticles->SetupAttachment(GetRootComponent());
}


void AProjectile::BeginPlay()
{
  Super::BeginPlay();
}


void AProjectile::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
  if (GetOwner() && GetOwner() == OtherActor) return;
  if (IsBlocked()) return;

  UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
  ExecuteGetHit(SweepResult);
}

