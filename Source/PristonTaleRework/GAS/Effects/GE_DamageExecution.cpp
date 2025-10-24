#include "GE_DamageExecution.h"
#include "GAS/AttributesSets/BasicAttributeSet.h"
#include "AbilitySystemComponent.h"

// Struct para capturar attributes
struct FDamageStatics
{
    DECLARE_ATTRIBUTE_CAPTUREDEF(MinPowerAttack);
    DECLARE_ATTRIBUTE_CAPTUREDEF(MaxPowerAttack);
    DECLARE_ATTRIBUTE_CAPTUREDEF(IncomingDamage);
    

    FDamageStatics()
    {
        // Capturar MinAttackPower do SOURCE (atacante)
        DEFINE_ATTRIBUTE_CAPTUREDEF(UBasicAttributeSet, MinPowerAttack, Source, false);

        // Capturar MaxAttackPower do SOURCE (atacante)
        DEFINE_ATTRIBUTE_CAPTUREDEF(UBasicAttributeSet, MaxPowerAttack, Source, false);

        // Capturar IncomingDamage do TARGET (defensor)
        DEFINE_ATTRIBUTE_CAPTUREDEF(UBasicAttributeSet, IncomingDamage, Target, false);
    }
};

static const FDamageStatics& DamageStatics()
{
    static FDamageStatics Statics;
    return Statics;
}

UGE_DamageExecution::UGE_DamageExecution()
{
    // Registrar attributes para captura
    RelevantAttributesToCapture.Add(DamageStatics().MinPowerAttackDef);
    RelevantAttributesToCapture.Add(DamageStatics().MaxPowerAttackDef);
    RelevantAttributesToCapture.Add(DamageStatics().IncomingDamageDef);
}

void UGE_DamageExecution::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                                 FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
    const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();

    // Obter ASCs
    UAbilitySystemComponent* SourceASC = ExecutionParams.GetSourceAbilitySystemComponent();
    UAbilitySystemComponent* TargetASC = ExecutionParams.GetTargetAbilitySystemComponent();

    if (!SourceASC || !TargetASC)
    {
        return;
    }

    // Obter tags do source e target
    const FGameplayTagContainer* SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
    const FGameplayTagContainer* TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

    FAggregatorEvaluateParameters EvaluateParameters;
    EvaluateParameters.SourceTags = SourceTags;
    EvaluateParameters.TargetTags = TargetTags;

    // Capturar MinAttackPower e MaxAttackPower do atacante
    float MinAttack = 0.0f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().MinPowerAttackDef,
        EvaluateParameters,
        MinAttack
    );

    float MaxAttack = 0.0f;
    ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(
        DamageStatics().MaxPowerAttackDef,
        EvaluateParameters,
        MaxAttack
    );
    
    // Calcular dano aleatório entre Min e Max
    float RawDamage = FMath::RandRange(MinAttack, MaxAttack);
    
    const FGameplayTag MultTag = FGameplayTag::RequestGameplayTag(FName("Combat.DamageMultiplier"));
    bool bHasMultiplier = false;
    float Multiplier = Spec.GetSetByCallerMagnitude(MultTag, bHasMultiplier);
    
    UE_LOG(LogTemp, Warning, TEXT("Tag: %s  | HasMultiplier: %d  | Multiplier: %.3f"),
        *MultTag.ToString(), bHasMultiplier ? 1 : 0, Multiplier);

    // usar fallback se necessário
    if (!bHasMultiplier)
    {
        UE_LOG(LogTemp, Warning, TEXT("SetByCaller não encontrado neste Spec — verifique onde foi definido."));
        Multiplier = 1.0f;
    }
    
    RawDamage *= Multiplier;
    
    UE_LOG(LogTemp, Warning, TEXT("Calculated damage: %.1f (Min: %.1f, Max: %.1f)"),
        RawDamage, MinAttack, MaxAttack);

    // Aplicar dano no IncomingDamage (o AttributeSet vai processar defesa)
    OutExecutionOutput.AddOutputModifier(
        FGameplayModifierEvaluatedData(
            DamageStatics().IncomingDamageProperty,
            EGameplayModOp::Additive,
            RawDamage
        )
    );
}
