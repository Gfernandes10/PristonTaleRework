#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "CombatEventSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnEnemyDefeatedEvent, AActor*, DefeatedEnemy, int32, ExperiencePoints);

UCLASS()
class PRISTONTALEREWORK_API UCombatEventSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Combat Events")
	FOnEnemyDefeatedEvent OnEnemyDefeated;

	UFUNCTION(BlueprintCallable, Category = "Combat Events")
	void BroadcastEnemyDefeated(AActor* DefeatedEnemy, int32 ExperiencePoints);
};
