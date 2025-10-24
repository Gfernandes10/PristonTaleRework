// GA_AreaAttack.h
#pragma once

#include "CoreMinimal.h"
#include "Abilities/GameplayAbility.h"
#include "GA_AreaAttack.generated.h"

UENUM(BlueprintType)
enum class EAreaShape : uint8
{
	Circle UMETA(DisplayName = "Circle"),
	Cone UMETA(DisplayName = "Cone")
};

UCLASS()
class UGA_AreaAttack : public UGameplayAbility
{
	GENERATED_BODY()

public:
	UGA_AreaAttack();

	virtual void ActivateAbility(
		const FGameplayAbilitySpecHandle Handle,
		const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo,
		const FGameplayEventData* TriggerEventData) override;

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Attack")
	float AreaRadius = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Attack")
	EAreaShape AreaShape = EAreaShape::Circle;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Attack")
	float ConeAngle = 90.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Attack")
	TSubclassOf<UGameplayEffect> DamageEffect;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Attack")
	float DamageMultiplier = 1.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Attack")
	UAnimMontage* AttackMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Attack")
	TEnumAsByte<ECollisionChannel> EnemyCollisionChannel = ECC_Pawn;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Attack")
	float AttackRange = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Area Attack")
	float MovementCheckInterval = 0.1f;
	
	UFUNCTION()
	void OnMontageCompleted();

	UFUNCTION()
	void OnMontageInterrupted();

private:
	void ApplyDamageToEnemiesInArea(const FVector& OriginLocation);
	TArray<AActor*> FindEnemiesInArea(const FVector& OriginLocation);
	bool IsInCone(const FVector& OriginLocation, const FVector& ForwardVector, const FVector& TargetLocation) const;

	TWeakObjectPtr<AActor> TargetActor;
	FTimerHandle MovementCheckTimer;

	void CheckDistanceAndAttack();
	void ExecuteAttack();
	void OnMovingToTarget(AActor* Target);
	void OnAttackExecuted();

public:
	virtual void EndAbility(const FGameplayAbilitySpecHandle Handle,
				   const FGameplayAbilityActorInfo* ActorInfo,
				   const FGameplayAbilityActivationInfo ActivationInfo,
				   bool bReplicateEndAbility,
				   bool bWasCancelled) override;

};
