// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "GAS/AttributesSets/StatsAttributeSet.h"

APlayerCharacter::APlayerCharacter()
{
	// Create the camera boom component
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));

	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->SetUsingAbsoluteRotation(true);
	CameraBoom->TargetArmLength = 800.f;
	CameraBoom->SetRelativeRotation(FRotator(-60.f, 0.f, 0.f));
	CameraBoom->bDoCollisionTest = false;

	// Create the camera component
	TopDownCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("TopDownCamera"));

	TopDownCameraComponent->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
	TopDownCameraComponent->bUsePawnControlRotation = false;

	// Activate ticking in order to update the cursor every frame.
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	StatsAttributeSet = CreateDefaultSubobject<UStatsAttributeSet>(TEXT("StatsAttributeSet"));
}

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent && StatsAttributeSet)
	{
		AbilitySystemComponent->SetNumericAttributeBase(
			UStatsAttributeSet::GetStrengthAttribute(), InitialStrength);
		AbilitySystemComponent->SetNumericAttributeBase(
			UStatsAttributeSet::GetIntelligenceAttribute(), InitialIntelligence);
		AbilitySystemComponent->SetNumericAttributeBase(
			UStatsAttributeSet::GetVitalityAttribute(), InitialVitality);
		AbilitySystemComponent->SetNumericAttributeBase(
			UStatsAttributeSet::GetAgilityAttribute(), InitialAgility);
		AbilitySystemComponent->SetNumericAttributeBase(
			UStatsAttributeSet::GetLevelAttribute(), InitialLevel);
		
		if (StatsToAttributesEffect)
		{
			FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
			EffectContext.AddSourceObject(this);
            
			FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
				StatsToAttributesEffect, 1.0f, EffectContext);

			if (SpecHandle.IsValid())
			{
				AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
			}
		}
	}
}

void APlayerCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	// stub
}

void APlayerCharacter::LevelUp()
{
    if (!AbilitySystemComponent || !StatsAttributeSet) return;

    // Increase Level in 1
    const float CurrentLevel = StatsAttributeSet->GetLevel();
    AbilitySystemComponent->SetNumericAttributeBase(
        UStatsAttributeSet::GetLevelAttribute(), CurrentLevel + 1.0f);

    // Add stats points for distribution
    const float CurrentPoints = StatsAttributeSet->GetAvailableStatPoints();
    AbilitySystemComponent->SetNumericAttributeBase(
        UStatsAttributeSet::GetAvailableStatPointsAttribute(), 
        CurrentPoints + PointsPerLevel);

    UE_LOG(LogTemp, Log, TEXT("Level Up! New Level: %.0f | Available Stats Points: %.0f"), 
        CurrentLevel + 1.0f, CurrentPoints + PointsPerLevel);
}

bool APlayerCharacter::AddStatPoint(FGameplayTag StatTag, int32 Amount)
{
    if (!AbilitySystemComponent || !StatsAttributeSet || Amount <= 0) 
        return false;

    const float AvailablePoints = StatsAttributeSet->GetAvailableStatPoints();
    if (AvailablePoints < Amount) 
        return false;

    // Identify which attribute to modify
    FGameplayAttribute AttributeToModify;
    
    if (StatTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Data.Stats.Strength")))
        AttributeToModify = UStatsAttributeSet::GetStrengthAttribute();
    else if (StatTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Data.Stats.Intelligence")))
        AttributeToModify = UStatsAttributeSet::GetIntelligenceAttribute();
    else if (StatTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Data.Stats.Vitality")))
        AttributeToModify = UStatsAttributeSet::GetVitalityAttribute();
    else if (StatTag.MatchesTagExact(FGameplayTag::RequestGameplayTag("Data.Stats.Agility")))
        AttributeToModify = UStatsAttributeSet::GetAgilityAttribute();
    else
        return false;

    // Remove available points
    AbilitySystemComponent->SetNumericAttributeBase(
        UStatsAttributeSet::GetAvailableStatPointsAttribute(), 
        AvailablePoints - Amount);

    // Add to chosen stats
    const float CurrentValue = AbilitySystemComponent->GetNumericAttribute(AttributeToModify);
    AbilitySystemComponent->SetNumericAttributeBase(AttributeToModify, CurrentValue + Amount);

    UE_LOG(LogTemp, Log, TEXT("Added %d point(s) to %s"), Amount, *StatTag.ToString());
    
    return true;
}

float APlayerCharacter::GetAvailableStatPoints() const
{
    return StatsAttributeSet ? StatsAttributeSet->GetAvailableStatPoints() : 0.0f;
}
