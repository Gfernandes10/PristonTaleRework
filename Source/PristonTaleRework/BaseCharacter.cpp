// Fill out your copyright notice in the Description page of Project Settings.


#include "BaseCharacter.h"

#include "PristonTaleRework.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GAS/AttributesSets/BasicAttributeSet.h"

// Sets default values
ABaseCharacter::ABaseCharacter()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	// Create Ability System Component
	AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(AscReplicationMode);

	// Set size for player capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// Don't rotate character to camera direction
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.f, 640.f, 0.f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	//Add the basic attribute set
	BasicAttributeSet = CreateDefaultSubobject<UBasicAttributeSet>(TEXT("BasicAttributeeSet"));

}

void ABaseCharacter::InitializeDefaultBasicAttributes()
{
	if (AbilitySystemComponent && DefaultBasicAttributes)
	{
		FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
		FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(DefaultBasicAttributes, 1, EffectContext);
		if (SpecHandle.IsValid())
		{			
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.DefaultBasicAttributes.MaxHealth"), DefaultMaxHealthAttribute);
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.DefaultBasicAttributes.MaxMana"), DefaultMaxManaAttribute);
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.DefaultBasicAttributes.Health"), DefaultHealthAttribute);
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.DefaultBasicAttributes.Mana"), DefaultManaAttribute);
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.DefaultBasicAttributes.MinPowerAttack"), DefaultMinPowerAttack);
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.DefaultBasicAttributes.MaxPowerAttack"), DefaultMaxPowerAttack);
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.DefaultBasicAttributes.Defense"), DefaultDefense);
			SpecHandle.Data->SetSetByCallerMagnitude(FGameplayTag::RequestGameplayTag("Data.DefaultBasicAttributes.DefenseRate"), DefaultDefenseRate);
			AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
		}		
	}
}
void ABaseCharacter::ReviveCharacter()
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    // 1. Restaurar atributos
    InitializeDefaultBasicAttributes();
	

    // 5. Remover tags de morte
    FGameplayTagContainer TagsToRemove;
    TagsToRemove.AddTag(FGameplayTag::RequestGameplayTag("Character.State.Dead"));
    AbilitySystemComponent->RemoveActiveEffectsWithTags(TagsToRemove);
    AbilitySystemComponent->RemoveLooseGameplayTag(FGameplayTag::RequestGameplayTag("Character.State.Dead"));

	// Add combat active tag
	AddGameplayTagToSelf(FGameplayTag::RequestGameplayTag("Combat.CanAttack.Enemy"));
	

	
}

// Called when the game starts or when spawned
void ABaseCharacter::BeginPlay()
{
	Super::BeginPlay();

	InitializeDefaultBasicAttributes();

	// Grant Basic Effects
	for (TSubclassOf<UGameplayEffect> EffectClass : BasicEffects)
	{
		if (EffectClass)
		{
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			EffectContext.AddSourceObject(this);

			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
				EffectClass, 1.0f, EffectContext);

			if (SpecHandle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
	// Grant Basic Abilities
	for (TSubclassOf<UGameplayAbility> AbilityClass : BasicAbilities)
	{
		if (AbilityClass)
		{
			AbilitySystemComponent->GiveAbility(
				FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this)
				);
		}
	}
}

// Called every frame
void ABaseCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ABaseCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ABaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

void ABaseCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	if (AbilitySystemComponent)
	{
		AbilitySystemComponent->InitAbilityActorInfo(this, this);
	}
}

UAbilitySystemComponent* ABaseCharacter::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}
void ABaseCharacter::AddGameplayTagToSelf(FGameplayTag TagToAdd)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->AddLooseGameplayTag(TagToAdd);
		UE_LOG(LogPristonTaleRework, Warning, TEXT("Tag adicionada: %s"), *TagToAdd.ToString());
	}
}

bool ABaseCharacter::ExecuteAttackOnTarget(AActor* TargetActor)
{
	FGameplayEventData Payload;
	Payload.Target = TargetActor;

	FGameplayTag AbilityTag = GetCurrentAttackTag();

	if (UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(TargetActor))
	{
		static const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Dead"));
		if (TargetASC->HasMatchingGameplayTag(DeadTag))
		{
			UE_LOG(LogPristonTaleRework, Log, TEXT("Target está morto, não atacar: %s"), *TargetActor->GetName());
			return false;
		}
	}
	int32 NumActivated = AbilitySystemComponent->HandleGameplayEvent(AbilityTag, &Payload);

	if (NumActivated > 0)
	{
		UE_LOG(LogPristonTaleRework, Log, TEXT("Ataque executado com sucesso! Abilities ativadas: %d"), NumActivated);
		return true;
	}
	UE_LOG(LogPristonTaleRework, Log, TEXT("Nenhuma ability foi ativada para a tag: %s"), *AbilityTag.ToString());
	return false;
}

void ABaseCharacter::RemoveGameplayTagFromSelf(FGameplayTag TagToRemove)
{
	if (UAbilitySystemComponent* ASC = GetAbilitySystemComponent())
	{
		ASC->RemoveLooseGameplayTag(TagToRemove);
		UE_LOG(LogPristonTaleRework, Warning, TEXT("Tag removida: %s"), *TagToRemove.ToString());
	}
}
FGameplayTag ABaseCharacter::GetCurrentAttackTag() const
{
	if (CurrentAttackTag.IsValid())
	{
		return CurrentAttackTag;
	}

	// Fallback padrão
	return FGameplayTag::RequestGameplayTag(TEXT("Ability.Attack.Melee"));
}