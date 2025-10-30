// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "AbilitySystemInterface.h"
#include "AbilitySystemComponent.h"
#include "BaseCharacter.generated.h"

UCLASS()
class PRISTONTALEREWORK_API ABaseCharacter : public ACharacter, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ABaseCharacter();
	
	

	// Ability System Component
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitiesSystem")
	UAbilitySystemComponent* AbilitySystemComponent;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "AbilitiesSystem")
	class UBasicAttributeSet* BasicAttributeSet;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AbilitiesSystem")
	EGameplayEffectReplicationMode AscReplicationMode = EGameplayEffectReplicationMode::Mixed;

	// Default health attribute to apply to the character
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|DefaultBasicAttributes")
	TSubclassOf<class UGameplayEffect> DefaultBasicAttributes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|DefaultBasicAttributes")
	float DefaultHealthAttribute = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|DefaultBasicAttributes")
	float DefaultMaxHealthAttribute = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|DefaultBasicAttributes")
	float DefaultManaAttribute = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|DefaultBasicAttributes")
	float DefaultMaxManaAttribute = 100.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|DefaultBasicAttributes")
	float DefaultMinPowerAttack = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|DefaultBasicAttributes")
	float DefaultMaxPowerAttack = 10.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|DefaultBasicAttributes")
	float DefaultDefense = 2.f;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GAS|DefaultBasicAttributes")
	float DefaultDefenseRate = 0.3f;	

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Abilities|Basic")
	TArray<TSubclassOf<UGameplayAbility>> BasicAbilities;

	UPROPERTY(EditDefaultsOnly, Category = "GAS|Effects|Basic")
	TArray<TSubclassOf<UGameplayEffect>> BasicEffects;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	void InitializeDefaultBasicAttributes();

public:		
	UFUNCTION(BlueprintCallable, Category = "Character State")
	void ReviveCharacter();
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	virtual class UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Character State")
	void OnReviveStart();

	UFUNCTION(BlueprintImplementableEvent, Category = "Character State")
	void OnReviveComplete();
};
