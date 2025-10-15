// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "StatsAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class PRISTONTALEREWORK_API UStatsAttributeSet : public UAttributeSet
{
	GENERATED_BODY()

public:

	UStatsAttributeSet();
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_Strength)
	FGameplayAttributeData Strength;
	ATTRIBUTE_ACCESSORS_BASIC(UStatsAttributeSet, Strength);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_Intelligence)	
	FGameplayAttributeData Intelligence;
	ATTRIBUTE_ACCESSORS_BASIC(UStatsAttributeSet, Intelligence);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_Vitality)
	FGameplayAttributeData Vitality;
	ATTRIBUTE_ACCESSORS_BASIC(UStatsAttributeSet, Vitality);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_Agility)
	FGameplayAttributeData Agility;
	ATTRIBUTE_ACCESSORS_BASIC(UStatsAttributeSet, Agility);
	
	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_AvailableStatPoints)
	FGameplayAttributeData AvailableStatPoints;
	ATTRIBUTE_ACCESSORS_BASIC(UStatsAttributeSet, AvailableStatPoints);

	UPROPERTY(BlueprintReadOnly, Category = "Attributes", ReplicatedUsing=OnRep_Level)
	FGameplayAttributeData Level;
	ATTRIBUTE_ACCESSORS_BASIC(UStatsAttributeSet, Level);

public:
	UFUNCTION()
	void OnRep_Strength(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UStatsAttributeSet, Strength, OldValue);
	}
	UFUNCTION()
	void OnRep_Intelligence(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UStatsAttributeSet, Intelligence, OldValue);
	}
	UFUNCTION()
	void OnRep_Vitality(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UStatsAttributeSet, Vitality, OldValue);
	}
	UFUNCTION()
	void OnRep_Agility(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UStatsAttributeSet, Agility, OldValue);
	}
	UFUNCTION()
	void OnRep_AvailableStatPoints(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UStatsAttributeSet, AvailableStatPoints, OldValue);
	}
	UFUNCTION()
	void OnRep_Level(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UStatsAttributeSet, Level, OldValue);
	}
	
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
};
