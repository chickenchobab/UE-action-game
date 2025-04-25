// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Weapon.h"
#include "Characters/SlashCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Components/SphereComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Interfaces/HitInterface.h"
#include "NiagaraComponent.h"

AWeapon::AWeapon()
{
	WeaponBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Weapon Box"));
  WeaponBox->SetupAttachment(GetRootComponent());
  WeaponBox->SetCollisionEnabled(ECollisionEnabled::NoCollision);
  WeaponBox->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Overlap);
  // Just to overlap with character mesh, not the capsule.
  WeaponBox->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Ignore);

  BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
  BoxTraceStart->SetupAttachment(GetRootComponent());
  BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
  BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void AWeapon::BeginPlay()
{
	Super::BeginPlay();

  WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AWeapon::OnBoxOverlap);
}

void AWeapon::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator, bool bPlayEquipSound)
{
  SetOwner(NewOwner);
  SetInstigator(NewInstigator);

  ItemState = EItemState::EIS_Equipped;
	AttachMeshToSocket(InParent, InSocketName);
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
  if (EmbersEffect)
  {
    EmbersEffect->Deactivate();
  }
}

void AWeapon::AttachMeshToSocket(USceneComponent* InParent, const FName& InSocketName)
{
	FAttachmentTransformRules TransformRules(EAttachmentRule::SnapToTarget, true);
  ItemMesh->AttachToComponent(InParent, TransformRules, InSocketName);
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

void AWeapon::OnBoxOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
  FHitResult BoxHit;

  BoxTrace(BoxHit);

  if (GetOwner() && BoxHit.GetActor() && !IsOnSameSide(BoxHit.GetActor()))
  {
    UE_LOG(LogTemp, Warning, TEXT("Actor : %s"), *BoxHit.GetActor()->GetName());
    UGameplayStatics::ApplyDamage(BoxHit.GetActor(), Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
    ExecuteGetHit(BoxHit);
    ActorsToIgnore.AddUnique(BoxHit.GetActor()); // Ignore multiple collision during one swing.
    CreateFields(BoxHit.ImpactPoint);
  }
}

void AWeapon::BoxTrace(FHitResult& BoxHit)
{
  const FVector Start = BoxTraceStart->GetComponentLocation();
  const FVector End = BoxTraceEnd->GetComponentLocation();

  UKismetSystemLibrary::BoxTraceSingle(
    this,
    Start,
    End,
    BoxTraceExtent,
    BoxTraceStart->GetComponentRotation(),
    UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
    false,
    ActorsToIgnore,
    bShowDebugBox ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
    BoxHit,
    true /* Add <this> to actors to ignore */
  );
}

void AWeapon::ExecuteGetHit(FHitResult &BoxHit)
{
  if (IHitInterface* HitInterface = Cast<IHitInterface>(BoxHit.GetActor()))
  {
    HitInterface->Execute_GetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, GetOwner());
  }
}


bool AWeapon::IsOnSameSide(AActor* OtherActor)
{
  if (GetOwner()->Tags.IsEmpty() || OtherActor->Tags.IsEmpty()) return false;
  return GetOwner()->Tags[0] == OtherActor->Tags[0];
}

