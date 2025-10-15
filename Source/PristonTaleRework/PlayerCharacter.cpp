// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacter.h"
#include "UObject/ConstructorHelpers.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Materials/Material.h"
#include "Engine/World.h"
#include "GAS/AttributesSets/StatsAttributeSet.h"
#include "GAS/AttributesSets/BasicAttributeSet.h"
#include "Kismet/GameplayStatics.h"
#include "PlayerSaveGame.h"
#include "Tables/ExperienceTableRow.h"

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

void APlayerCharacter::UpdateBasicAttributesBaseOnStats()
{
	if (AbilitySystemComponent && StatsToAttributesEffect)
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

void APlayerCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (AbilitySystemComponent && StatsAttributeSet)
	{
		if (DoesSaveGameExist())
		{
			LoadGame(SaveSlotName);
		}
		else
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
			UpdateBasicAttributesBaseOnStats();
		}		
	}

	// Add Tags if needs Health/Mana Regen
	if (AbilitySystemComponent && BasicAttributeSet)
	{
		static const FGameplayTag NeedsHealthRegenTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Regen.HP"));
		static const FGameplayTag NeedsManaRegenTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Regen.MP"));

		if (BasicAttributeSet->GetHealth() < BasicAttributeSet->GetMaxHealth())
		{
			AbilitySystemComponent->AddLooseGameplayTag(NeedsHealthRegenTag);
		}

		if (BasicAttributeSet->GetMana() < BasicAttributeSet->GetMaxMana())
		{
			AbilitySystemComponent->AddLooseGameplayTag(NeedsManaRegenTag);
		}
	}
	// Apply Health/Mana Regen Effect
    if (AbilitySystemComponent && HealthManaRegenEffect)
    {
        FGameplayEffectContextHandle EffectContext = AbilitySystemComponent->MakeEffectContext();
        EffectContext.AddSourceObject(this);
    
        FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(
            HealthManaRegenEffect, 1.0f, EffectContext);
    
        if (SpecHandle.IsValid())
        {
            RegenEffectHandle = AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());
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

	SaveGame(SaveSlotName);
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

	SaveGame(SaveSlotName);
	
    return true;
}

float APlayerCharacter::GetAvailableStatPoints() const
{
    return StatsAttributeSet ? StatsAttributeSet->GetAvailableStatPoints() : 0.0f;
}

void APlayerCharacter::SaveGame(FString SlotName)
{
	if (!StatsAttributeSet) return;

	UPlayerSaveGame* SaveGameInstance = Cast<UPlayerSaveGame>(
		UGameplayStatics::CreateSaveGameObject(UPlayerSaveGame::StaticClass()));

	if (SaveGameInstance)
	{
		// Save Stats
		SaveGameInstance->Strength = FMath::RoundToInt(StatsAttributeSet->GetStrength());
		SaveGameInstance->Intelligence = FMath::RoundToInt(StatsAttributeSet->GetIntelligence());
		SaveGameInstance->Vitality = FMath::RoundToInt(StatsAttributeSet->GetVitality());
		SaveGameInstance->Agility = FMath::RoundToInt(StatsAttributeSet->GetAgility());
		SaveGameInstance->Level = FMath::RoundToInt(StatsAttributeSet->GetLevel());
		SaveGameInstance->AvailableStatPoints = FMath::RoundToInt(StatsAttributeSet->GetAvailableStatPoints());
		
		// Save Experience
		SaveGameInstance->CurrentExperience = CurrentExperience;
		
		// Save Position
		SaveGameInstance->PlayerPosition = GetActorLocation();
		SaveGameInstance->PlayerRotation = GetActorRotation();

		// Save on disk
		if (UGameplayStatics::SaveGameToSlot(SaveGameInstance, SlotName, 0))
		{
			UE_LOG(LogTemp, Log, TEXT("Game saved successfully!"));
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to save game!"));
		}
	}
}

void APlayerCharacter::LoadGame(FString SlotName)
{
	if (!AbilitySystemComponent || !StatsAttributeSet) return;

	UPlayerSaveGame* LoadGameInstance = Cast<UPlayerSaveGame>(
		UGameplayStatics::LoadGameFromSlot(SlotName, 0));
	if (!LoadGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("Not possible to open save slot: %s"), *SlotName);
		return;
	}

	if (LoadGameInstance)
	{
		// Load Stats
		AbilitySystemComponent->SetNumericAttributeBase(
			UStatsAttributeSet::GetStrengthAttribute(), LoadGameInstance->Strength);
		AbilitySystemComponent->SetNumericAttributeBase(
			UStatsAttributeSet::GetIntelligenceAttribute(), LoadGameInstance->Intelligence);
		AbilitySystemComponent->SetNumericAttributeBase(
			UStatsAttributeSet::GetVitalityAttribute(), LoadGameInstance->Vitality);
		AbilitySystemComponent->SetNumericAttributeBase(
			UStatsAttributeSet::GetAgilityAttribute(), LoadGameInstance->Agility);
		AbilitySystemComponent->SetNumericAttributeBase(
			UStatsAttributeSet::GetLevelAttribute(), LoadGameInstance->Level);
		AbilitySystemComponent->SetNumericAttributeBase(
			UStatsAttributeSet::GetAvailableStatPointsAttribute(), LoadGameInstance->AvailableStatPoints);
		
		// Load Experience
		CurrentExperience = LoadGameInstance->CurrentExperience;
		
		// Load Position
		SetActorLocation(LoadGameInstance->PlayerPosition);
		SetActorRotation(LoadGameInstance->PlayerRotation);

		// Update Basic Attributes based on Stats
		UpdateBasicAttributesBaseOnStats();

		UE_LOG(LogTemp, Log, TEXT("Game loaded successfully!"));
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to load game!"));
	}
}
bool APlayerCharacter::DoesSaveGameExist() const
{
	return UGameplayStatics::DoesSaveGameExist(SaveSlotName, 0);
}

void APlayerCharacter::DeleteSaveGame(FString SlotName)
{
    if (UGameplayStatics::DoesSaveGameExist(SlotName, 0))
    {
        if (UGameplayStatics::DeleteGameInSlot(SlotName, 0))
        {
            UE_LOG(LogTemp, Log, TEXT("Save game '%s' deleted successfully!"), *SlotName);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to delete save game '%s'"), *SlotName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("Save game '%s' does not exist"), *SlotName);
    }
}

void APlayerCharacter::DeleteSaveGameAndReset(FString SlotName)
{
    DeleteSaveGame(SlotName);
    
    // Reset para valores iniciais
    if (AbilitySystemComponent && StatsAttributeSet)
    {
        AbilitySystemComponent->SetNumericAttributeBase(UStatsAttributeSet::GetStrengthAttribute(), InitialStrength);
        AbilitySystemComponent->SetNumericAttributeBase(UStatsAttributeSet::GetIntelligenceAttribute(), InitialIntelligence);
        AbilitySystemComponent->SetNumericAttributeBase(UStatsAttributeSet::GetVitalityAttribute(), InitialVitality);
        AbilitySystemComponent->SetNumericAttributeBase(UStatsAttributeSet::GetAgilityAttribute(), InitialAgility);
        AbilitySystemComponent->SetNumericAttributeBase(UStatsAttributeSet::GetLevelAttribute(), InitialLevel);
        AbilitySystemComponent->SetNumericAttributeBase(UStatsAttributeSet::GetAvailableStatPointsAttribute(), 0);
        
        CurrentExperience = 0;
        
        UpdateBasicAttributesBaseOnStats();
    }
}

void APlayerCharacter::AddExperience(int32 Amount)
{
	if (!AbilitySystemComponent || !StatsAttributeSet || !ExperienceTable) return;

	CurrentExperience += Amount;
    
	int32 RequiredXP = GetExperienceForNextLevel();
    
	while (CurrentExperience >= RequiredXP && RequiredXP > 0)
	{
		CurrentExperience -= RequiredXP;
		LevelUp();
		RequiredXP = GetExperienceForNextLevel();
	}

	SaveGame(SaveSlotName);
}

int32 APlayerCharacter::GetExperienceForNextLevel() const
{
	if (!ExperienceTable || !StatsAttributeSet) return 0;

	int32 CurrentLevel = FMath::RoundToInt(StatsAttributeSet->GetLevel());
	int32 NextLevel = CurrentLevel + 1;
	FString RowName = FString::FromInt(NextLevel);
    
	FExperienceTableRow* Row = ExperienceTable->FindRow<FExperienceTableRow>(
		FName(*RowName), TEXT(""));
    
	return Row ? Row->ExperienceRequired : 0;
}

float APlayerCharacter::GetExperienceProgress() const
{
	int32 RequiredXP = GetExperienceForNextLevel();
	return RequiredXP > 0 ? (float)CurrentExperience / RequiredXP : 0.0f;
}