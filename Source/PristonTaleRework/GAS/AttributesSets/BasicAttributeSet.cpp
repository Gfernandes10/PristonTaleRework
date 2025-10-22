// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"

UBasicAttributeSet::UBasicAttributeSet()
{
	Health = 100.f;
	MaxHealth = 100.f;
	Mana = 100.f;
	MaxMana = 100.f;
	MinPowerAttack = 2.f;
	MaxPowerAttack = 10.f;
	Defense = 2.f;
	DefenseRate = 0.3f;
}

void UBasicAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Health, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MaxHealth, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Mana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MaxMana, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MinPowerAttack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, MaxPowerAttack, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, Defense, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UBasicAttributeSet, DefenseRate, COND_None, REPNOTIFY_Always);
}

void UBasicAttributeSet::ManageRegenTag(UAbilitySystemComponent* ASC, const FGameplayTag& Tag, bool bShouldHaveTag)
{
    if (!ASC) return;

    const bool bHasTag = ASC->HasMatchingGameplayTag(Tag);

    if (bShouldHaveTag && !bHasTag)
    {
        ASC->AddLooseGameplayTag(Tag);
    }
    else if (!bShouldHaveTag && bHasTag)
    {
        ASC->RemoveLooseGameplayTag(Tag);
    }
}

void UBasicAttributeSet::PreAttributeChange(const FGameplayAttribute& Attribute, float& NewValue)
{
	Super::PreAttributeChange(Attribute, NewValue);
}

void UBasicAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

    if (Attribute == GetMaxHealthAttribute())
    {
        const float HealthPercentage = (NewValue > 0.0f)
            ? GetHealth() / NewValue
            : 1.0f;

        float NewHealth = FMath::Clamp(NewValue * HealthPercentage, 0.0f, NewValue);
    	SetHealth(NewHealth);

        UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
        static const FGameplayTag HealthTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Regen.Health"));
        ManageRegenTag(ASC, HealthTag, GetHealth() < NewValue);
    }
    else if (Attribute == GetMaxManaAttribute())
    {
        const float ManaPercentage = (NewValue > 0.0f)
            ? GetMana() / NewValue
            : 1.0f;

        float NewMana = FMath::Clamp(NewValue * ManaPercentage, 0.0f, NewValue);
    	SetMana(NewMana);

        UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
        static const FGameplayTag ManaTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Regen.Mana"));
        ManageRegenTag(ASC, ManaTag, GetMana() < NewValue);
    }
}

void UBasicAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
    Super::PostGameplayEffectExecute(Data);

    UAbilitySystemComponent* ASC = Data.Target.AbilityActorInfo->AbilitySystemComponent.Get();
    if (!ASC) return;

    static const FGameplayTag NeedsHealthRegenTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Regen.Health"));
    static const FGameplayTag NeedsManaRegenTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Regen.Mana"));

    if (Data.EvaluatedData.Attribute == GetHealthAttribute())
    {
        // SetHealth(FMath::Clamp(GetHealth(), 0.0f, GetMaxHealth()));
        ManageRegenTag(ASC, NeedsHealthRegenTag, GetHealth() < GetMaxHealth());
    }
    else if (Data.EvaluatedData.Attribute == GetManaAttribute())
    {
        // SetMana(FMath::Clamp(GetMana(), 0.0f, GetMaxMana()));
        ManageRegenTag(ASC, NeedsManaRegenTag, GetMana() < GetMaxMana());
    }
}
