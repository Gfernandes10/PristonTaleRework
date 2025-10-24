// MMC_MeleeDamage.cpp
#include "MMC_Damage.h"
#include "GAS/AttributesSets/BasicAttributeSet.h" // Substitua pelo seu AttributeSet

UOMMC_MeleeDamage::UOMMC_MeleeDamage()
{
	// Capture MinPowerAttack do Source (Atacante)
	MinPowerAttackDef = FGameplayEffectAttributeCaptureDefinition(
		UBasicAttributeSet::GetMinPowerAttackAttribute(),
		EGameplayEffectAttributeCaptureSource::Source,
		true
	);

	// Capture MaxPowerAttack do Source (Atacante)
	MaxPowerAttackDef = FGameplayEffectAttributeCaptureDefinition(
		UBasicAttributeSet::GetMaxPowerAttackAttribute(),
		EGameplayEffectAttributeCaptureSource::Source,
		true
	);

	RelevantAttributesToCapture.Add(MinPowerAttackDef);
	RelevantAttributesToCapture.Add(MaxPowerAttackDef);
}

float UOMMC_MeleeDamage::CalculateBaseMagnitude_Implementation(const FGameplayEffectSpec& Spec) const
{
	const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = SourceTags;
	EvaluationParameters.TargetTags = TargetTags;

	float MinPower = 0.f;
	GetCapturedAttributeMagnitude(MinPowerAttackDef, Spec, EvaluationParameters, MinPower);

	float MaxPower = 0.f;
	GetCapturedAttributeMagnitude(MaxPowerAttackDef, Spec, EvaluationParameters, MaxPower);

	// Calcular dano aleatório entre Min e Max
	float Damage = FMath::RandRange(MinPower, MaxPower);

	// Aplicar multiplicador (SetByCaller)
	const FGameplayTag MultTag = FGameplayTag::RequestGameplayTag(FName("Combat.DamageMultiplier"));
	float Multiplier = Spec.GetSetByCallerMagnitude(MultTag, false, 1.0f);

	return Damage * Multiplier;
}
