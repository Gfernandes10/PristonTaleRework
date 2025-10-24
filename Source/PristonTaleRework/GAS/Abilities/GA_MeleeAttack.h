// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_MeleeAttack.generated.h"

/**
 * 
 */
UCLASS()
class PRISTONTALEREWORK_API UGA_MeleeAttack : public UGameplayAbility
{
	GENERATED_BODY()

	UGA_MeleeAttack();

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	float AttackRange = 200.0f;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	float MovementCheckInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Combat|Combo")
	TArray<UAnimMontage*> ComboMontages;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|Combo")
	float ComboResetTime = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|VFX")
	UParticleSystem* HitParticle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Combat|VFX")
	USoundBase* AttackSound;

	
private:
	FTimerHandle MovementCheckTimer;
	FTimerHandle ComboResetTimer;
	TWeakObjectPtr<AActor> TargetActor;
	int32 CurrentComboIndex = 0;
	ECollisionResponse OriginalPawnCollisionResponse;
	void DisableCollisionWithEnemies();
	void RestoreCollision();
	
protected:
	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnAttackExecuted(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat")
	void OnMovingToTarget(AActor* Target);

	UFUNCTION(BlueprintImplementableEvent, Category = "Combat|Combo")
	void OnComboIndexChanged(int32 ComboIndex, int32 MaxComboCount);

private:
	void CheckDistanceAndAttack();
	
	void ExecuteAttack();

	UFUNCTION()
	void OnMontageCompleted();
    
	UFUNCTION()
	void OnMontageInterrupted();

	void ResetCombo();

	UAnimMontage* GetCurrentComboMontage() const;
	
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
