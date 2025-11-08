// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_BuffAbility.generated.h"

UCLASS()
class PRISTONTALEREWORK_API UGA_BuffAbility : public UGameplayAbility
{
	GENERATED_BODY()

	UGA_BuffAbility();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Buff")
	UAnimMontage* BuffMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Buff")
	TSubclassOf<UGameplayEffect> BuffEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Buff|VFX")
	UParticleSystem* BuffParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Buff|VFX")
	USoundBase* BuffSound;

	UPROPERTY()
	FActiveGameplayEffectHandle ActiveBuffHandle;
	
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();


	void ApplyBuffEffect();

protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Buff")
	void OnBuffApplied();

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle,
								const FGameplayAbilityActorInfo* ActorInfo,
								const FGameplayAbilityActivationInfo ActivationInfo,
								const FGameplayEventData* TriggerEventData) override;

	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
						  const FGameplayAbilityActorInfo* ActorInfo,
						  const FGameplayAbilityActivationInfo ActivationInfo,
						  bool bReplicateEndAbility,
						  bool bWasCancelled) override;
};
