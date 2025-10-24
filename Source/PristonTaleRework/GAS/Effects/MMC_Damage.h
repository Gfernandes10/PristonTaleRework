// MMC_MeleeDamage.h
#pragma once

#include "CoreMinimal.h"
#include "GameplayModMagnitudeCalculation.h"
#include "MMC_Damage.generated.h"

UCLASS()
class UOMMC_MeleeDamage : public UGameplayModMagnitudeCalculation
{
	GENERATED_BODY()

public:
	UOMMC_MeleeDamage();

	virtual float CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const override;

private:
	FGameplayEffectAttributeCaptureDefinition MinPowerAttackDef;
	FGameplayEffectAttributeCaptureDefinition MaxPowerAttackDef;
};