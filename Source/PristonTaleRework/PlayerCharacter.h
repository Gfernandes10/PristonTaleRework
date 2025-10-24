// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "Tables/EGameSaveSlots.h"
#include "PlayerCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PRISTONTALEREWORK_API APlayerCharacter : public ABaseCharacter
{
	GENERATED_BODY()

private:

	/** Top down camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	class UCameraComponent* TopDownCameraComponent;

	/** Camera boom positioning the camera above the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	class USpringArmComponent* CameraBoom;
	
public: 
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitiesSystem")
	class UStatsAttributeSet* StatsAttributeSet;

protected:
	// Gameplay Effect that converts Stats → Atributos
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
	TSubclassOf<UGameplayEffect> StatsToAttributesEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Effects")
	TMap<FGameplayTag, TSubclassOf<UGameplayEffect>> StatPointChangeEffects;
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Effects")
	TSubclassOf<UGameplayEffect> HealthManaRegenEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Stats")
	int32 InitialStrength = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Stats")
	int32 InitialIntelligence = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Stats")
	int32 InitialVitality = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Stats")
	int32 InitialAgility = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Stats")
	int32 InitialLevel = 1;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "GAS|Stats")
	int32 PointsPerLevel = 5;
	
	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities|Tier1")
	TArray<TSubclassOf<UGameplayAbility>> Tier1Abilities;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities|Tier2")
	TArray<TSubclassOf<UGameplayAbility>> Tier2Abilities;
	
	UPROPERTY(EditDefaultsOnly, Category = "Experience")
	UDataTable* ExperienceTable;

	UPROPERTY(BlueprintReadOnly, Category = "Experience")
	int32 CurrentExperience = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Save System")
	EGameSaveSlots CurrentSaveSlot = EGameSaveSlots::SlotA;
	
public:

	/** Constructor */
	APlayerCharacter();

	/** Initialization */
	virtual void BeginPlay() override;

	/** Update */
	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void LevelUp();

	UFUNCTION(BlueprintCallable, Category = "Stats")
	bool AddStatPoint(FGameplayTag StatTag, int32 Amount = 1);

	UFUNCTION(BlueprintPure, Category = "Stats")
	float GetAvailableStatPoints() const;

	UFUNCTION(BlueprintCallable, Category = "Stats")
	void UpdateBasicAttributesBaseOnStats();
	

	/** Returns the camera component **/
	FORCEINLINE class UCameraComponent* GetTopDownCameraComponent() const { return TopDownCameraComponent; }

	/** Returns the Camera Boom component **/
	FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void SaveGame(EGameSaveSlots Slot);

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void LoadGame(EGameSaveSlots Slot);

	UFUNCTION(BlueprintPure, Category = "SaveGame")
	bool DoesSaveGameExist(EGameSaveSlots Slot) const;

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void DeleteSaveGame(EGameSaveSlots Slot);

	UFUNCTION(BlueprintCallable, Category = "SaveGame")
	void DeleteSaveGameAndReset(EGameSaveSlots Slot);

	UFUNCTION(BlueprintCallable, Category = "Experience")
	void AddExperience(int32 Amount);

	UFUNCTION(BlueprintPure, Category = "Experience")
	int32 GetExperienceForNextLevel() const;

	UFUNCTION(BlueprintPure, Category = "Experience")
	float GetExperienceProgress() const; // Returns 0.0 to 1.0 for UI

private:
	FActiveGameplayEffectHandle RegenEffectHandle;

	FString GetSlotNameFromEnum(EGameSaveSlots Slot) const;
	
};
