// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AttributeSet.h"
#include "AbilitySystemComponent.h"
#include "BasicAttributeSet.generated.h"

/**
 * 
 */
UCLASS()
class PRISTONTALEREWORK_API UBasicAttributeSet : public UAttributeSet
{
	GENERATED_BODY()
private:
	float OldMaxHealth = 0.0f;
	float OldMaxMana = 0.0f;
public:

	UBasicAttributeSet();
	
	UPROPERTY(BlueprintReadOnly, Category="GAS|Attributes", ReplicatedUsing=OnRep_Health)
	FGameplayAttributeData Health;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Health);
	
	UPROPERTY(BlueprintReadOnly, Category="GAS|Attributes", ReplicatedUsing=OnRep_MaxHealth)
	FGameplayAttributeData MaxHealth;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxHealth);
	
	UPROPERTY(BlueprintReadOnly, Category="GAS|Attributes", ReplicatedUsing=OnRep_Mana)
	FGameplayAttributeData Mana;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Mana);
	
	UPROPERTY(BlueprintReadOnly, Category="GAS|Attributes", ReplicatedUsing=OnRep_MaxMana)
	FGameplayAttributeData MaxMana;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxMana);
	
	UPROPERTY(BlueprintReadOnly, Category="GAS|Attributes", ReplicatedUsing=OnRep_MinPowerAttackk)
	FGameplayAttributeData MinPowerAttack;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MinPowerAttack);
	
	UPROPERTY(BlueprintReadOnly, Category="GAS|Attributes", ReplicatedUsing=OnRep_MaxPowerAttack)
	FGameplayAttributeData MaxPowerAttack;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, MaxPowerAttack);
	
	UPROPERTY(BlueprintReadOnly, Category="GAS|Attributes", ReplicatedUsing=OnRep_Defense)
	FGameplayAttributeData Defense;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, Defense);
	
	UPROPERTY(BlueprintReadOnly, Category="GAS|Attributes", ReplicatedUsing=OnRep_DefenseRate)
	FGameplayAttributeData DefenseRate;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, DefenseRate);

	UPROPERTY(BlueprintReadOnly, Category = "GAS|Attributes")
	FGameplayAttributeData IncomingDamage;
	ATTRIBUTE_ACCESSORS_BASIC(UBasicAttributeSet, IncomingDamage)

public:
	UFUNCTION()
	void OnRep_Health(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Health, OldValue);
	}
	UFUNCTION()
	void OnRep_MaxHealth(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxHealth, OldValue);
	}
	UFUNCTION()
	void OnRep_Mana(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Mana, OldValue);
	}
	UFUNCTION()
	void OnRep_MaxMana(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxMana, OldValue);
	}
	UFUNCTION()
	void OnRep_MinPowerAttackk(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MinPowerAttack, OldValue);
	}
	UFUNCTION()
	void OnRep_MaxPowerAttack(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, MaxPowerAttack, OldValue);
	}
	UFUNCTION()
	void OnRep_Defense(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, Defense, OldValue);
	}
	UFUNCTION()
	void OnRep_DefenseRate(const FGameplayAttributeData& OldValue) const
	{
		GAMEPLAYATTRIBUTE_REPNOTIFY(UBasicAttributeSet, DefenseRate, OldValue);
	}

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data) override;

	virtual void PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue) override;

	virtual void PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue) override;
	

private:
        void ManageRegenTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag, bool bShouldHaveTag);
	
	
};
