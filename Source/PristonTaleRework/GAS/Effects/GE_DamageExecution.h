#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "GE_DamageExecution.generated.h"

UCLASS()
class PRISTONTALEREWORK_API UGE_DamageExecution : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UGE_DamageExecution();

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
									   FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
