// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Weapon.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "NiagaraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Characters/SlashCharacter.h"
#include "Interfaces/HitInterface.h"


AWeapon::AWeapon()
{
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
  WeaponBox->SetupAttachment(GetRootComponent());
  WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
  // Just to overlap with character mesh, not the capsule.
  WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator, bool bPlayEquipSound)
{
  SetOwner(NewOwner);
  SetInstigator(NewInstigator);
  AttachMeshToSocket(InParent, InSocketName);

  ItemState = EItemState::EIS_Equipped;
  ResetActorsToIgnore();
  if (Sphere)
  {
    Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  }

  if (bPlayEquipSound && EquipSound)
  {
    UGameplayStatics::PlaySoundAtLocation(
      this,
      EquipSound,
      GetActorLocation()
    );
  }
  if (ItemEffect)
  {
    ItemEffect->Deactivate();
  }
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
  ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
}


void AWeapon::DetachMeshFromSocket()
{
  FDetachmentTransformRules TransformRules(EDetachmentRule::KeepWorld, true);
  ItemMesh->DetachFromComponent(TransformRules);
}


void AWeapon::SetWeaponBoxCollisionEnabled(ECollisionEnabled::Type CollisionEnabled)
{
  WeaponBox->SetCollisionEnabled(CollisionEnabled);
}


void AWeapon::ResetActorsToIgnore()
{
	ActorsToIgnore.Empty();
  if (GetOwner())
  {
    ActorsToIgnore.Add(GetOwner());
  }
}

void AWeapon::BeginPlay()
{
  Super::BeginPlay();
	WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}

void AWeapon::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
  if (OtherActor)
  {
    UE_LOG(LogTemp, Warning, TEXT("Hit actor : %s"), *OtherActor->GetName());
  }
  if (GetOwner() && GetOwner() == OtherActor) return;

  if (!IsActorIgnored(OtherActor) && IsOwnerOpposite(OtherActor))
  {
    ActorsToIgnore.Add(OtherActor);
    TryApplyDamage(OtherActor);
    ExecuteGetHit(OtherActor, SweepResult.ImpactPoint);
  }
}


void AWeapon::TryApplyDamage(AActor * OtherActor)
{
  if (ABaseCharacter* HitCharacter = Cast<ABaseCharacter>(OtherActor))
  {
    if (!HitCharacter->IsParrying() || !IsCharacterFacingWeapon(HitCharacter))
    {
      UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
    }
  }
}

void AWeapon::ExecuteGetHit(AActor* OtherActor, FVector ImpactPoint)
{
  if (IHitInterface* HitInterface = Cast<IHitInterface>(OtherActor))
  {
    HitInterface->Execute_GetHit(OtherActor, ImpactPoint, GetOwner());
  }
}

bool AWeapon::IsOwnerOpposite(AActor* OtherActor)
{
  if (ABaseCharacter* OwnerCharacter = Cast<ABaseCharacter>(GetOwner()))
  {
    return OwnerCharacter->IsOpposite(OtherActor);
  }
  return false;
}


bool AWeapon::IsActorIgnored(AActor* OtherActor)
{
  for (AActor* Actor : ActorsToIgnore)
  {
    if (Actor == OtherActor) return true;
  }
  return false;
}

bool AWeapon::IsCharacterFacingWeapon(ABaseCharacter* HitCharacter)
{
  if (HitCharacter == nullptr) return false;

  FVector CharacterForward = (HitCharacter->GetActorForwardVector()).GetSafeNormal();
  FVector CharacterToWeapon = (GetActorLocation() - HitCharacter->GetActorLocation()).GetSafeNormal();
  float FacingAngle = FMath::RadiansToDegrees(FMath::Acos(FVector::DotProduct(CharacterForward, CharacterToWeapon)));

  return FacingAngle <= 30.f;
}

