
#include "GA_MeleeAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/Character.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "Components/CapsuleComponent.h"

UGA_MeleeAttack::UGA_MeleeAttack()
{
	InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
	NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;

	FAbilityTriggerData TriggerData;
	TriggerData.TriggerTag = FGameplayTag::RequestGameplayTag("Ability.Attack.Melee");
	AbilityTriggers.Add(TriggerData);
}

void UGA_MeleeAttack::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
									  const FGameplayAbilityActorInfo* ActorInfo,
									  const FGameplayAbilityActivationInfo ActivationInfo,
									  const FGameplayEventData* TriggerEventData)
{
	if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	GetWorld()->GetTimerManager().ClearTimer(ComboResetTimer);

	if (ComboMontages.Num() == 0)
	{
		UE_LOG(LogTemp, Error, TEXT("GA_MeleeAttack: None ComboMontage configured!"));
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}
	
	// Obtain target
	if (TriggerEventData && TriggerEventData->Target)
	{
		TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());
	}

	if (!TargetActor.IsValid())
	{
		EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
		return;
	}

	// Start  moving to target
	if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
	{
		if (AController* Controller = Character->GetController())
		{
			// Usar UAIBlueprintHelperLibrary para compatibilidade
			UAIBlueprintHelperLibrary::SimpleMoveToActor(Controller, TargetActor.Get());
		}
	}
	
	OnMovingToTarget(TargetActor.Get());
	
	// Start timer to verify  distance
	GetWorld()->GetTimerManager().SetTimer(
		MovementCheckTimer,
		this,
		&UGA_MeleeAttack::CheckDistanceAndAttack,
		MovementCheckInterval,
		true
	);
}

void UGA_MeleeAttack::CheckDistanceAndAttack()
{
	if (!TargetActor.IsValid())
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (!Avatar)
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
		return;
	}

	float Distance = FVector::Dist(Avatar->GetActorLocation(), TargetActor->GetActorLocation());

	if (Distance <= AttackRange)
	{
		// Stop moving
		if (ACharacter* Character = Cast<ACharacter>(Avatar))
		{
			if (AController* Controller = Character->GetController())
			{
				Controller->StopMovement();
			}
		}

		// Stop timer
		GetWorld()->GetTimerManager().ClearTimer(MovementCheckTimer);

		// Executt attack
		ExecuteAttack();
	}
}

void UGA_MeleeAttack::ExecuteAttack()
{
    // Rotate character to target
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (Avatar && TargetActor.IsValid())
    {
        FVector Direction = TargetActor->GetActorLocation() - Avatar->GetActorLocation();
        Direction.Z = 0;
        FRotator NewRotation = Direction.Rotation();
        Avatar->SetActorRotation(NewRotation);
    }

	DisableCollisionWithEnemies();
	
    // Apply damage effect
    if (DamageEffect && TargetActor.IsValid())
    {
        FGameplayEffectContextHandle EffectContext = GetAbilitySystemComponentFromActorInfo()->MakeEffectContext();
        EffectContext.AddSourceObject(this);

        FGameplayEffectSpecHandle SpecHandle = GetAbilitySystemComponentFromActorInfo()->MakeOutgoingSpec(
            DamageEffect,
            GetAbilityLevel(),
            EffectContext
        );

        if (SpecHandle.IsValid())
        {
        	if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor.Get()))
        	{
        		GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data.Get(), TargetASC);
        	}
        }
    }
	
	OnAttackExecuted(TargetActor.Get());
	
    // Play attack montage
	UAnimMontage* ComboMontageToPlay = GetCurrentComboMontage();
    if (ComboMontageToPlay)
    {
    	OnComboIndexChanged(CurrentComboIndex, ComboMontages.Num());
    	
    	UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ComboMontageToPlay
		);

    	if (Task)
    	{
    		Task->OnCompleted.AddDynamic(this, &UGA_MeleeAttack::OnMontageCompleted);
    		Task->OnInterrupted.AddDynamic(this, &UGA_MeleeAttack::OnMontageInterrupted);
    		Task->OnCancelled.AddDynamic(this, &UGA_MeleeAttack::OnMontageInterrupted);
    		Task->ReadyForActivation();
    	}
    	else
    	{
    		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    	}
    }
    else
    {
    	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }


}

void UGA_MeleeAttack::OnMontageCompleted()
{
	RestoreCollision();
	// Next animation montage
	CurrentComboIndex = (CurrentComboIndex + 1) % ComboMontages.Num();
	// Start combo reset timer
	GetWorld()->GetTimerManager().SetTimer(
		ComboResetTimer,
		this,
		&UGA_MeleeAttack::ResetCombo,
		ComboResetTime,
		false);
	
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MeleeAttack::OnMontageInterrupted()
{
	RestoreCollision();
	ResetCombo();
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}

void UGA_MeleeAttack::EndAbility(const FGameplayAbilitySpecHandle Handle,
								 const FGameplayAbilityActorInfo* ActorInfo,
								 const FGameplayAbilityActivationInfo ActivationInfo,
								 bool bReplicateEndAbility,
								 bool bWasCancelled)
{
	GetWorld()->GetTimerManager().ClearTimer(MovementCheckTimer);
	TargetActor.Reset();

	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
void UGA_MeleeAttack::ResetCombo()
{
	CurrentComboIndex = 0;
	OnComboIndexChanged(CurrentComboIndex, ComboMontages.Num());
}
UAnimMontage* UGA_MeleeAttack::GetCurrentComboMontage() const
{
	if (ComboMontages.IsValidIndex(CurrentComboIndex))
	{
		return ComboMontages[CurrentComboIndex];
	}
	return nullptr;
}
void UGA_MeleeAttack::DisableCollisionWithEnemies()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (ACharacter* Character = Cast<ACharacter>(Avatar))
	{
		UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
		if (Capsule)
		{
			// Save original collision response
			OriginalPawnCollisionResponse = Capsule->GetCollisionResponseToChannel(ECC_Pawn);
            
			// Ignore colission with other pawns
			Capsule->SetCollisionResponseToChannel(ECC_Pawn, ECR_Ignore);
			
		}
	}
}
void UGA_MeleeAttack::RestoreCollision()
{
	AActor* Avatar = GetAvatarActorFromActorInfo();
	if (ACharacter* Character = Cast<ACharacter>(Avatar))
	{
		UCapsuleComponent* Capsule = Character->GetCapsuleComponent();
		if (Capsule)
		{
			// Restore collision response
			Capsule->SetCollisionResponseToChannel(ECC_Pawn, OriginalPawnCollisionResponse);
			
		}
	}
}