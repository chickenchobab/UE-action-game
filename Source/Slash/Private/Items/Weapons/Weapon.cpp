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


void AWeapon::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{

}

void AWeapon::ExecuteGetHit(const FHitResult &BoxHit)
{
  if (IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor()))
  {
    HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());
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