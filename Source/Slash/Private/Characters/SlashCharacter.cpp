// Fill out your copyright notice in the Description page of Project Settings.


#include "Characters/SlashCharacter.h"
#include "EnhancedInputSubsystems.h"
// #include "InputMappingContext.h"
// #include "InputAction.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "Animation/AnimMontage.h"
#include "Perception/AIPerceptionStimuliSourceComponent.h"
#include "Perception/AISense_Sight.h"

#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "GroomComponent.h"
#include "Components/BoxComponent.h"

#include "Items/Item.h"
#include "Items/Weapons/Weapon.h"
#include "Items/Soul.h"
#include "Items/Treasure.h"
#include "HUD/SlashHUD.h"
#include "HUD/SlashOverlay.h"
#include "Components/AttributeComponent.h"

ASlashCharacter::ASlashCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 400.f, 0.f);

	GetMesh()->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	GetMesh()->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Visibility, ECollisionResponse::ECR_Block);
	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetRootComponent());
	CameraBoom->TargetArmLength = 300.f;
	CameraBoom->bUsePawnControlRotation = true;

	ViewCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ViewCamera"));
	ViewCamera->SetupAttachment(CameraBoom);

	Hair = CreateDefaultSubobject<UGroomComponent>(TEXT("Hair"));
	Hair->SetupAttachment(GetMesh());
	Hair->AttachmentName = FString("head");

	Eyebrows = CreateDefaultSubobject<UGroomComponent>(TEXT("Eyebrows"));
	Eyebrows->SetupAttachment(GetMesh());
	Eyebrows->AttachmentName = FString("head");
}


void ASlashCharacter::Tick(float DeltaTime)
{
	if (Attributes && SlashOverlay)
	{
		Attributes->RegenStamina(DeltaTime);
		SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
	}

	if (GetVelocity().Size() > 0.f)
	{
		MovingTime += DeltaTime;
	}
	else
	{
		MovingTime = 0.f;
	}
}


void ASlashCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Move);
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Look);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Triggered, this, &ASlashCharacter::Jump);
		EnhancedInputComponent->BindAction(EkeyAction, ETriggerEvent::Triggered, this, &ASlashCharacter::EkeyPressed);
		EnhancedInputComponent->BindAction(AttackAction, ETriggerEvent::Started, this, &ASlashCharacter::LeftMouseClicked);
		EnhancedInputComponent->BindAction(DodgeAction, ETriggerEvent::Triggered, this, &ASlashCharacter::RightMouseClicked);
	}
}


float ASlashCharacter::TakeDamage(float DamageAmount, struct FDamageEvent const &DamageEvent, class AController *EventInstigator, AActor *DamageCauser)
{
	HandleDamage(DamageAmount);
  UpdateHealthBar();
  return DamageAmount;
}


void ASlashCharacter::Jump()
{
	if (IsUnoccupied())
	{
		Super::Jump();
	}
}

void ASlashCharacter::GetHit_Implementation(const FVector& ImpactPoint, AActor* Hitter, bool bReact)
{
	Super::GetHit_Implementation(ImpactPoint, Hitter, bReact);

	if (IsAlive())
	{
		ActionState = EActionState::EAS_HitReacting;
	}
	SetWeaponCollisionEnabled(ECollisionEnabled::NoCollision);
	StopAttackMontage();
}


void ASlashCharacter::SetOverlappingItem(AItem* Item)
{
	OverlappingItem = Item;
}


void ASlashCharacter::GetSoul(ASoul* Soul)
{
	if (Soul && Attributes && SlashOverlay)
	{
		Attributes->AddSouls(Soul->GetSouls());
		SlashOverlay->SetSouls(Attributes->GetSouls());
	}
}


void ASlashCharacter::GetGold(ATreasure* Treasure)
{
	if (Treasure && Attributes)
	{
		Attributes->AddGold(Treasure->GetGold());
		SlashOverlay->SetGold(Attributes->GetGold());
	}
}


void ASlashCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(SlashCharacterContext, 0);
		}

		if (ASlashHUD* SlashHUD = Cast<ASlashHUD>(PlayerController->GetHUD()))
		{
			SlashOverlay = SlashHUD->GetSlashOverlay();
			if (SlashOverlay && Attributes)
			{
				SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
				SlashOverlay->SetStaminaBarPercent(1.f);
				SlashOverlay->SetGold(0);
				SlashOverlay->SetSouls(0);
			}
		}
	}

	Tags.Add(FName("Ally"));
}


void ASlashCharacter::Attack()
{
	Super::Attack();

	if (CanAttack())
	{
		SetActorRotation(RecentInputRotation);
		
		if (MovingTime >= 3.f)
		{
			ActionState = EActionState::EAS_DashAttacking;
			PlaySpecialAttackMontage();
		}
		else
		{
			ActionState = EActionState::EAS_Attacking;
			PlayAttackMontage(true);
		}
	}
	else if (IsInCombo())
	{
		++ComboCount;
		UE_LOG(LogTemp, Warning, TEXT("in combo"));
		GetWorldTimerManager().ClearTimer(ComboWindowTimer);
		GetWorldTimerManager().SetTimer(ComboWindowTimer, this, &ASlashCharacter::ResetComboCount, ComboWindow);
	}
}


void ASlashCharacter::Die()
{
	Super::Die();

	ActionState = EActionState::EAS_Dead;
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}


void ASlashCharacter::Dodge()
{
	Super::Dodge();
	
	if (CanDodge())
	{
		SetActorRotation(RecentInputRotation);
		PlayDodgeMontage();
		ActionState = EActionState::EAS_Dodging;
		if (Attributes && SlashOverlay)
		{
			Attributes->UseStamina(Attributes->GetDodgeCost());
			SlashOverlay->SetStaminaBarPercent(Attributes->GetStaminaPercent());
		} 
	}
}


bool ASlashCharacter::CanAttack()
{
	bool bMontageIsPlaying = false;
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance()) 
	{	
		bMontageIsPlaying = AnimInstance->Montage_IsPlaying(AttackMontage);
	}

	return !bMontageIsPlaying &&
		ActionState == EActionState::EAS_Unoccupied && 
		CharacterState != ECharacterState::ECS_Unequipped;
}

void ASlashCharacter::AttackEnd()
{
	Super::AttackEnd();

	ActionState = EActionState::EAS_Unoccupied;
	if (ComboCount == 0)
	{
		StopAttackMontage(0.5f);
	}
	else
	{
		--ComboCount;
	}
}

void ASlashCharacter::DodgeEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}


void ASlashCharacter::HandleDamage(float DamageAmount)
{
	Super::HandleDamage(DamageAmount);
}

void ASlashCharacter::Move(const FInputActionValue& Value)
{
	if (Controller != nullptr && ActionState == EActionState::EAS_Unoccupied)
	{
		FVector2d MovementVector = Value.Get<FVector2d>(); 

		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation = FRotator(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		const FVector Direction = ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X;
		RecentInputRotation = Direction.Rotation();

		// AddMovementInput(ForwardDirection, MovementVector.Y);
		// AddMovementInput(RightDirection, MovementVector.X);
		AddMovementInput(Direction, 1.f);
	}
}

void ASlashCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASlashCharacter::EkeyPressed(const FInputActionValue& Value)
{
	if (AWeapon* OverlappingWeapon = Cast<AWeapon>(OverlappingItem))
	{
		EquipWeapon(OverlappingWeapon);
	}
	else if (EquippedWeapon)
	{
		if (CanDisarm())
		{
			Disarm();
		}
		else if (CanArm())
		{
			Arm();
		}
	}
}

void ASlashCharacter::LeftMouseClicked(const FInputActionValue& Value)
{
	Attack();
}

void ASlashCharacter::RightMouseClicked(const FInputActionValue& Value)
{
	Dodge();
}

void ASlashCharacter::PlayEquipMontage(const FName &SectionName)
{
	PlayMontageSection(EquipMontage, SectionName);
}

bool ASlashCharacter::CanDisarm()
{
	return ActionState == EActionState::EAS_Unoccupied && 
		CharacterState != ECharacterState::ECS_Unequipped;
}

bool ASlashCharacter::CanArm()
{
	return ActionState == EActionState::EAS_Unoccupied &&
		CharacterState == ECharacterState::ECS_Unequipped;
}

void ASlashCharacter::AttachWeaponToBack()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("SpineSocket"));
	}
}

void ASlashCharacter::AttachWeaponToHand()
{
	if (EquippedWeapon)
	{
		EquippedWeapon->AttachMeshToSocket(GetMesh(), FName("RightHandSocket"));
	}
}

void ASlashCharacter::EquipEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::HitReactEnd()
{
	ActionState = EActionState::EAS_Unoccupied;
}

void ASlashCharacter::ResetComboCount()
{
	// Should be called when a combo end
	ComboCount = 0;
}

void ASlashCharacter::EquipWeapon(AWeapon* Weapon)
{
	Weapon->Equip(GetMesh(), FName("RightHandSocket"), this, this, true);
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	EquippedWeapon = Weapon;
	OverlappingItem = nullptr;
}

void ASlashCharacter::Disarm()
{
	PlayEquipMontage(FName("Disarm"));
	CharacterState = ECharacterState::ECS_Unequipped;
	ActionState = EActionState::EAS_Equipping;
}

void ASlashCharacter::Arm()
{
	PlayEquipMontage(FName("Arm"));
	CharacterState = ECharacterState::ECS_EquippedOneHandedWeapon;
	ActionState = EActionState::EAS_Equipping;
}

void ASlashCharacter::UpdateHealthBar()
{
  if (SlashOverlay && Attributes)
  {
    SlashOverlay->SetHealthBarPercent(Attributes->GetHealthPercent());
  }
}

bool ASlashCharacter::CanDodge()
{
	if (!IsUnoccupied()) return false;
	return Attributes && Attributes->GetStamina() >= Attributes->GetDodgeCost();
}


bool ASlashCharacter::IsInCombo()
{
	if (!IsEquipped()) return false;
	if (IsAttacking()) return true;
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance()) 
	{	
		return AnimInstance->Montage_IsPlaying(AttackMontage);
	}
	return false;
}
