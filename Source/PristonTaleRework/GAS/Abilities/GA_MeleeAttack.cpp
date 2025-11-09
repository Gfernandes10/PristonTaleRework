
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
	GetAbilitySystemComponentFromActorInfo()->AddLooseGameplayTag(
		FGameplayTag::RequestGameplayTag("Ability.Attack.Active")
	);
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
        	const FGameplayTag MultTag = FGameplayTag::RequestGameplayTag(FName("Combat.DamageMultiplier"));
        	if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
        	{
        		Spec->SetSetByCallerMagnitude(MultTag, DamageMultiplier);
        		if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor.Get()))
        		{
        			GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*Spec, TargetASC);
        		}
        	}
        }
    }
	
	OnAttackExecuted(TargetActor.Get());
	
    // Play attack montage
	UAnimMontage* ComboMontageToPlay = GetCurrentComboMontage();
	if (ComboMontageToPlay)
	{
		//UE_LOG(LogTemp, Warning, TEXT("🎬 Playing montage: %s"), *ComboMontageToPlay->GetName());
		OnComboIndexChanged(CurrentComboIndex, ComboMontages.Num());

		UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
			this,
			NAME_None,
			ComboMontageToPlay,
			1.0f,
			NAME_None,
			false,
			0.0f,
			0.0f,
			true
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
			//UE_LOG(LogTemp, Error, TEXT("❌ Failed to create PlayMontageAndWait task"));
			EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
		}
	}
	else
	{
		//UE_LOG(LogTemp, Error, TEXT("❌ No montage configured, ending ability immediately"));
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
	}


}

void UGA_MeleeAttack::OnMontageCompleted()
{
	//UE_LOG(LogTemp, Warning, TEXT("✅ Montage COMPLETED"));
	RestoreCollision();
	CurrentComboIndex = (CurrentComboIndex + 1) % ComboMontages.Num();
	GetWorld()->GetTimerManager().SetTimer(
		ComboResetTimer,
		this,
		&UGA_MeleeAttack::ResetCombo,
		ComboResetTime,
		false);

	//UE_LOG(LogTemp, Warning, TEXT("📤 Calling EndAbility from OnMontageCompleted"));
	EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_MeleeAttack::OnMontageInterrupted()
{
	//UE_LOG(LogTemp, Warning, TEXT("⚠️ Montage INTERRUPTED"));
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
	//UE_LOG(LogTemp, Warning, TEXT("🛑 EndAbility called (Cancelled: %s)"), bWasCancelled ? TEXT("Yes") : TEXT("No"));
    
	GetWorld()->GetTimerManager().ClearTimer(MovementCheckTimer);
	TargetActor.Reset();

	GetAbilitySystemComponentFromActorInfo()->RemoveLooseGameplayTag(
		FGameplayTag::RequestGameplayTag("Ability.Attack.Active")
	);
    
	Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
    
	//UE_LOG(LogTemp, Warning, TEXT("✅ EndAbility completed"));
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