// Copyright Epic Games, Inc. All Rights Reserved.

#include "PristonTaleReworkPlayerController.h"

#include "AbilitySystemGlobals.h"
#include "GameFramework/Pawn.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "PlayerCharacter.h"
#include "Engine/World.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "PristonTaleRework.h"

APristonTaleReworkPlayerController::APristonTaleReworkPlayerController()
{
	bIsTouch = false;
	bMoveToMouseCursor = false;

	// configure the controller
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;
	CachedDestination = FVector::ZeroVector;
	FollowTime = 0.f;
}

void APristonTaleReworkPlayerController::SetupInputComponent()
{
	// set up gameplay key bindings
	Super::SetupInputComponent();

	// Only set up input on local player controllers
	if (IsLocalPlayerController())
	{
		// Add Input Mapping Context
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}

		// Set up action bindings
		if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(InputComponent))
		{
			// Setup mouse input events
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Started, this, &APristonTaleReworkPlayerController::OnInputStarted);
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Triggered, this, &APristonTaleReworkPlayerController::OnSetDestinationTriggered);
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Completed, this, &APristonTaleReworkPlayerController::OnSetDestinationReleased);
			EnhancedInputComponent->BindAction(SetDestinationClickAction, ETriggerEvent::Canceled, this, &APristonTaleReworkPlayerController::OnSetDestinationReleased);

			// Setup right mouse click event
			EnhancedInputComponent->BindAction(RightClickAction, ETriggerEvent::Started, this, &APristonTaleReworkPlayerController::OnRightMouseClick);

			// Setup Shift + Right Click
			EnhancedInputComponent->BindAction(ShiftRightClickAction, ETriggerEvent::Started, this, &APristonTaleReworkPlayerController::OnShiftRightMouseClick);

			// Setup touch input events
			EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Started, this, &APristonTaleReworkPlayerController::OnInputStarted);
			EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Triggered, this, &APristonTaleReworkPlayerController::OnTouchTriggered);
			EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Completed, this, &APristonTaleReworkPlayerController::OnTouchReleased);
			EnhancedInputComponent->BindAction(SetDestinationTouchAction, ETriggerEvent::Canceled, this, &APristonTaleReworkPlayerController::OnTouchReleased);
		}
		else
		{
			UE_LOG(LogPristonTaleRework, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
		}
	}
}

void APristonTaleReworkPlayerController::OnInputStarted()
{
	if (bIsAutoAttacking)
	{
		StopAutoAttack();
	}
	StopMovement();
}

void APristonTaleReworkPlayerController::OnSetDestinationTriggered()
{
	// We flag that the input is being pressed
	FollowTime += GetWorld()->GetDeltaSeconds();
	
	// We look for the location in the world where the player has pressed the input
	FHitResult Hit;
	bool bHitSuccessful = false;
	if (bIsTouch)
	{
		bHitSuccessful = GetHitResultUnderFinger(ETouchIndex::Touch1, ECollisionChannel::ECC_Visibility, true, Hit);
	}
	else
	{
		bHitSuccessful = GetHitResultUnderCursor(ECollisionChannel::ECC_Visibility, true, Hit);
	}

	// If we hit a surface, cache the location
	if (bHitSuccessful)
	{
		CachedDestination = Hit.Location;
	}
	
	// Move towards mouse pointer or touch
	APawn* ControlledPawn = GetPawn();
	if (ControlledPawn != nullptr)
	{
		FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
		ControlledPawn->AddMovementInput(WorldDirection, 1.0, false);
	}
}

void APristonTaleReworkPlayerController::OnSetDestinationReleased()
{
	// If it was a short press
	if (FollowTime <= ShortPressThreshold)
	{
		// We move there and spawn some particles
		UAIBlueprintHelperLibrary::SimpleMoveToLocation(this, CachedDestination);
		UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, FXCursor, CachedDestination, FRotator::ZeroRotator, FVector(1.f, 1.f, 1.f), true, true, ENCPoolMethod::None, true);
	}

	FollowTime = 0.f;
}

// Triggered every frame when the input is held down
void APristonTaleReworkPlayerController::OnTouchTriggered()
{
	bIsTouch = true;
	OnSetDestinationTriggered();
}

void APristonTaleReworkPlayerController::OnTouchReleased()
{
	bIsTouch = false;
	OnSetDestinationReleased();
}

bool APristonTaleReworkPlayerController::VerifyHitResult(FHitResult& HitResult)
{
	FVector2D MousePosition;
	GetMousePosition(MousePosition.X, MousePosition.Y);

	FVector WorldLocation, WorldDirection;
	DeprojectScreenPositionToWorld(MousePosition.X, MousePosition.Y, WorldLocation, WorldDirection);

	FVector TraceEnd = WorldLocation + (WorldDirection * 10000.0f);

	TArray<FHitResult> HitResults;
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
	ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));
    
	GetWorld()->SweepMultiByObjectType(
		HitResults,
		WorldLocation,
		TraceEnd,
		FQuat::Identity,
		ObjectTypes,
		FCollisionShape::MakeSphere(100.0f)
	);
	
	UE_LOG(LogPristonTaleRework, Warning, TEXT("Hits encontrados: %d"), HitResults.Num());
    
	for (const FHitResult& Hit : HitResults)
	{
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Hit.GetActor()))
		{
			static const FGameplayTag CombatCanAttackEnemyTag = FGameplayTag::RequestGameplayTag(FName("Combat.CanAttack.Enemy"));
			if (TargetASC->HasMatchingGameplayTag(CombatCanAttackEnemyTag))
			{
				HitResult = Hit;
				UE_LOG(LogPristonTaleRework, Warning, TEXT("Inimigo válido encontrado: %s"), *Hit.GetActor()->GetName());
				return false;
			}
		}
	}
	UE_LOG(LogPristonTaleRework, Warning, TEXT("Nenhum inimigo válido encontrado"));
	return true;
}

void APristonTaleReworkPlayerController::OnRightMouseClick()
{
	if (bIsAutoAttacking)
	{
		StopAutoAttack();
	}

	FHitResult HitResult;
	if (VerifyHitResult(HitResult)) return;

	FGameplayEventData Payload;
	Payload.Target = HitResult.GetActor();
	
	ExecuteSingleAttack(HitResult.GetActor());
}
void APristonTaleReworkPlayerController::OnShiftRightMouseClick()
{
	FHitResult HitResult;
	
	if (VerifyHitResult(HitResult)) return;
	
	StartAutoAttack(HitResult.GetActor());
}
void APristonTaleReworkPlayerController::StartAutoAttack(AActor* Target)
{
	if (!Target)
	{
		return;
	}
	
	StopAutoAttack();

	bIsAutoAttacking = true;
	AutoAttackTarget = Target;
	
	
	ExecuteSingleAttack(Target);
	
	GetWorld()->GetTimerManager().SetTimer(
		AutoAttackCheckTimer,
		this,
		&APristonTaleReworkPlayerController::CheckAutoAttackConditions,
		0.1f, // Verificar a cada 0.5 segundos
		true
	);
}
void APristonTaleReworkPlayerController::StopAutoAttack()
{
	if (!bIsAutoAttacking)
	{
		return;
	}

	bIsAutoAttacking = false;
	GetWorld()->GetTimerManager().ClearTimer(AutoAttackCheckTimer);
	AutoAttackTarget.Reset();

	UE_LOG(LogPristonTaleRework, Warning, TEXT("Auto-attack stoped"));
}
void APristonTaleReworkPlayerController::CheckAutoAttackConditions()
{
	if (!bIsAutoAttacking)
	{
		return;
	}
	
	if (!AutoAttackTarget.IsValid())
	{
		UE_LOG(LogPristonTaleRework, Warning, TEXT("Target killed, stopping auto-attack"));
		StopAutoAttack();
		return;
	}
	
	AActor* Target = AutoAttackTarget.Get();
    
	// Opção 1: Verifiy if target still has "CanAttack.Enemy" tag
	// if (!Target->ActorHasTag("Combat.CanAttack.Enemy"))
	// {
	// 	UE_LOG(LogPristonTaleRework, Warning, TEXT("Target is no longer attackable, stopping auto-attack"));
	// 	StopAutoAttack();
	// 	return;
	// }


	APlayerCharacter* PlayerChar = GetPawn<APlayerCharacter>();
	if (UAbilitySystemComponent* ASC = PlayerChar->GetAbilitySystemComponent())
	{
		FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag("Cooldown.Attack.Melee");
		if (ASC->HasMatchingGameplayTag(CooldownTag))
		{
			return;
		}
	}
	// Ok, execute next attack
	ExecuteSingleAttack(Target);
}
void APristonTaleReworkPlayerController::ExecuteSingleAttack(AActor* Target)
{
	if (!Target)
	{
		return;
	}

	APlayerCharacter* PlayerChar = GetPawn<APlayerCharacter>();
	if (!PlayerChar)
	{
		return;
	}

	UAbilitySystemComponent* ASC = PlayerChar->GetAbilitySystemComponent();
	if (!ASC)
	{
		return;
	}

	FGameplayEventData Payload;
	Payload.Target = Target;

	FGameplayTag AbilityTag = PlayerChar->GetCurrentAttackTag();
    ASC->HandleGameplayEvent(AbilityTag, &Payload);
}