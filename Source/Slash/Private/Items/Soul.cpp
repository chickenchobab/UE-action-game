// Fill out your copyright notice in the Description page of Project Settings.


#include "Items/Soul.h"
#include "Interfaces/PickupInterface.h"

void ASoul::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
  if (!bAllowOverlapEvent) return;

  if (IPickupInterface* PickupInterface = Cast<IPickupInterface>(OtherActor))
  {
    PickupInterface->GetSoul(this);
  }
  SpawnPickupSystem();
  SpawnPickupSound();
  Destroy();
}
