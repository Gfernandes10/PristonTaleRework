#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PlayerSaveGame.generated.h"

UCLASS()
class PRISTONTALEREWORK_API UPlayerSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPlayerSaveGame();

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	FString SaveSlotName;

	UPROPERTY(VisibleAnywhere, Category = "SaveGame")
	uint32 UserIndex;

	// Player Stats
	UPROPERTY(VisibleAnywhere, Category = "Stats")
	int32 Strength;

	UPROPERTY(VisibleAnywhere, Category = "Stats")
	int32 Intelligence;

	UPROPERTY(VisibleAnywhere, Category = "Stats")
	int32 Vitality;

	UPROPERTY(VisibleAnywhere, Category = "Stats")
	int32 Agility;

	UPROPERTY(VisibleAnywhere, Category = "Stats")
	int32 Level;

	UPROPERTY(VisibleAnywhere, Category = "Stats")
	int32 AvailableStatPoints;

	// Secundary Attributes
	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float CurrentHealth;

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float CurrentMana;

	UPROPERTY(VisibleAnywhere, Category = "Attributes")
	float CurrentStamina;

	// Player Position
	UPROPERTY(VisibleAnywhere, Category = "Transform")
	FVector PlayerPosition;

	UPROPERTY(VisibleAnywhere, Category = "Transform")
	FRotator PlayerRotation;

	// Playe Progression
	UPROPERTY(VisibleAnywhere, Category = "Experience")
	int32 CurrentExperience = 0;

	UPROPERTY(VisibleAnywhere, Category = "Tags")
	TArray<FString> UnlockedAbilityTags;
};
