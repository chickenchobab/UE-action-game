// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Weapons/Armament.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

AArmament::AArmament()
{
  BoxTraceStart = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace Start"));
  BoxTraceStart->SetupAttachment(GetRootComponent());
  BoxTraceEnd = CreateDefaultSubobject<USceneComponent>(TEXT("Box Trace End"));
  BoxTraceEnd->SetupAttachment(GetRootComponent());
}


void AArmament::Equip(USceneComponent* InParent, FName InSocketName, AActor* NewOwner, APawn* NewInstigator, bool bPlayEquipSound)
{
  Super::Equip(InParent, InSocketName, NewOwner, NewInstigator, bPlayEquipSound);

  AttachMeshToSocket(InParent, InSocketName);
}

void AArmament::BeginPlay()
{
  Super::BeginPlay();

  WeaponBox->OnComponentBeginOverlap.AddDynamic(this, &AArmament::OnBoxOverlap);
}


void AArmament::OnBoxOverlap(UPrimitiveComponent *OverlappedComponent, AActor *OtherActor, UPrimitiveComponent *OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult &SweepResult)
{
  if (!IsOwnerOpposite(OtherActor)) 
  { 
    UE_LOG(LogTemp, Warning, TEXT("Box overlap : Not enemy(%s)->%s"), *OtherActor->GetName(), *OtherComp->GetName());
  }
  else
  {
    UE_LOG(LogTemp, Warning, TEXT("Box overlap : Enemy(%s)->%s"), *OtherActor->GetName(), *OtherComp->GetName());
  }
  
  FHitResult BoxHit;
  BoxTrace(BoxHit);

  if (!BoxHit.GetActor())
  {
    UE_LOG(LogTemp, Warning, TEXT("No hit actor"));
  }

  if (GetOwner() && BoxHit.GetActor())
  {
    UE_LOG(LogTemp, Warning, TEXT("Hit Actor(%s)->%s"), *BoxHit.GetActor()->GetName(), *BoxHit.GetComponent()->GetName());
    if (!IsOwnerOpposite(BoxHit.GetActor()))
    {
      UE_LOG(LogTemp, Warning, TEXT("No damage to ally"));
    }
    if (!IsBlocked())
    {
      UGameplayStatics::ApplyDamage(BoxHit.GetActor(), Damage, GetInstigator()->GetController(), this, UDamageType::StaticClass());
      ExecuteGetHit(BoxHit);
    }
    CreateFields(BoxHit.ImpactPoint);
  }
}


void AArmament::BoxTrace(FHitResult& BoxHit)
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
