// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "EnemyCharacter.generated.h"

/**
 * 
 */
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnAttackFinished);


UCLASS()
class PRISTONTALEREWORK_API AEnemyCharacter : public ABaseCharacter
{
	GENERATED_BODY()

	AEnemyCharacter();
	
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|Combat")
	float AttackRange = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|Combat")
	float DetectionRange = 1000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|Combat")
	float AttackCheckInterval = 0.1f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "AI|Combat")
	FGameplayTag AttackAbilityTag;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "AI|Combat")
	TWeakObjectPtr<AActor> TargetPlayer;
	
	UPROPERTY(BlueprintAssignable, Category = "Combat")
	FOnAttackFinished OnAttackFinished;
private:
	FTimerHandle AttackCheckTimer;

	void CheckPlayerDistance();

	void OnAttackTagChanged(FGameplayTag Tag, int32 NewCount);

public:
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void ExecuteAttack();
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	bool CanAttack();
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	bool GetPlayerTarget();


	
	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void StartFollowingPlayer(AActor* Player);

	UFUNCTION(BlueprintCallable, Category = "AI|Combat")
	void StopFollowingPlayer();
};
