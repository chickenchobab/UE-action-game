// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/MeleeWeapon.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

#include "Characters/BaseCharacter.h"

AMeleeWeapon::AMeleeWeapon()
{
  BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
  BoxTraceStart->SetupAttachment(GetRootComponent());
  BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
  BoxTraceEnd->SetupAttachment(GetRootComponent());
}

void AMeleeWeapon::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{ 
  FHitResult BoxHit;
  BoxTrace(BoxHit);

  if (GetOwner() && BoxHit.GetActor())
  {
    // UE_LOG(LogTemp, Warning, TEXT("Hit Actor(%s)->%s"), *BoxHit.GetActor()->GetName(), *BoxHit.GetComponent()->GetName());
    if (IsOwnerOpposite(BoxHit.GetActor()))
    {
      ExecuteGetHit(BoxHit.GetActor(), BoxHit.ImpactPoint, TryApplyDamage(BoxHit.GetActor()));
    }
    CreateFields(BoxHit.ImpactPoint);
  }
}


void AMeleeWeapon::BoxTrace(FHitResult& BoxHit)
{
  const FVector Start = BoxTraceStart->GetComponentLocation();
  const FVector End = BoxTraceEnd->GetComponentLocation();

  bShowDebugBox = true;
  UKismetSystemLibrary::BoxTraceSingle(
    this,
    End,
    Start,
    BoxTraceExtent,
    BoxTraceStart->GetComponentRotation(),
    UEngineTypes::ConvertToTraceType(ECollisionChannel::ECC_Visibility),
    false,
    ActorsToIgnore,
    bShowDebugBox ? EDrawDebugTrace::ForDuration : EDrawDebugTrace::None,
    BoxHit,
    true /* Add <this> to actors to ignore */
  );

  ActorsToIgnore.AddUnique(BoxHit.GetActor()); // Ignore multiple collision during one swing.
}
