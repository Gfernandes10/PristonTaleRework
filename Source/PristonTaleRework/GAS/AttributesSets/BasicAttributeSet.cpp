// Fill out your copyright notice in the Description page of Project Settings.


#include "BasicAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "PristonTaleRework.h"

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
	if (Attribute == GetDefenseRateAttribute())
	{
		NewValue = FMath::Clamp(NewValue, 0.0f, 0.7f); 
	}
}

void UBasicAttributeSet::PostAttributeChange(const FGameplayAttribute& Attribute, float OldValue, float NewValue)
{
    Super::PostAttributeChange(Attribute, OldValue, NewValue);

	UAbilitySystemComponent* ASC = GetOwningAbilitySystemComponent();
	if (!ASC) return;

	// Verifica se é um player
	AActor* OwnerActor = ASC->GetOwnerActor();
	APawn* OwnerPawn = Cast<APawn>(OwnerActor);
	bool bIsPlayer = OwnerPawn && OwnerPawn->IsPlayerControlled();
	
    if (Attribute == GetMaxHealthAttribute())
    {
        const float HealthPercentage = (NewValue > 0.0f)
            ? GetHealth() / NewValue
            : 1.0f;

        float NewHealth = FMath::Clamp(NewValue * HealthPercentage, 0.0f, NewValue);
    	SetHealth(NewHealth);
    	
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
    	
        static const FGameplayTag ManaTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Regen.Mana"));
        ManageRegenTag(ASC, ManaTag, GetMana() < NewValue);
    }
    else if (Attribute == GetHealthAttribute() && bIsPlayer && OldValue > NewValue)
    {
    	static const FGameplayTag HealthTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Regen.Health"));
    	ManageRegenTag(ASC, HealthTag, GetHealth() < GetMaxHealth());
    }
    else if (Attribute == GetManaAttribute() && bIsPlayer && OldValue > NewValue)
    {
    	static const FGameplayTag ManaTag = FGameplayTag::RequestGameplayTag(FName("Character.State.Regen.Mana"));
    	ManageRegenTag(ASC, ManaTag, GetMana() < GetMaxMana());
    }
}

// cpp
bool UBasicAttributeSet::PreGameplayEffectExecute(FGameplayEffectModCallbackData& Data)
{
	bool bParentReturn = Super::PreGameplayEffectExecute(Data);
	

	
	return bParentReturn;
	
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

	if (Data.EvaluatedData.Attribute == GetIncomingDamageAttribute())
	{
		static const FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag("Character.State.Dead");
    
		if (ASC && ASC->HasMatchingGameplayTag(DeadTag))
		{
			// Dead already
			SetIncomingDamage(0.0f);
			return;
		}
    
		float DamageValue = GetIncomingDamage();
		UE_LOG(LogPristonTaleRework, Log, TEXT("IncomingDamage: %.2f"), DamageValue);
    
		SetIncomingDamage(0.0f);
    
		if (DamageValue > 0.0f)
		{
			bool bDefenseApplied = FMath::FRand() <= GetDefenseRate();
			float FinalDamage = DamageValue;
        
			if (bDefenseApplied)
			{
				FinalDamage = FMath::Max(0.0f, DamageValue - GetDefense());
				UE_LOG(LogPristonTaleRework, Log, TEXT("Defesa Aplicada! Dano Original: %.2f | Dano Após Defesa: %.2f"), DamageValue, FinalDamage);
			}
			else
			{
				UE_LOG(LogPristonTaleRework, Log, TEXT("Defesa NÃO Aplicada! Dano Final: %.2f"), FinalDamage);
			}
        
			float NewHealth = GetHealth() - FinalDamage;
			SetHealth(FMath::Clamp(NewHealth, 0.0f, GetMaxHealth()));

			if (GetHealth() <= 0.0f)
			{
				// Handle death logic here (e.g., broadcast a death event)
				if (ASC)
				{
					ASC->AddLooseGameplayTag(DeadTag);
					UE_LOG(LogPristonTaleRework, Log, TEXT("%s morreu!"), *Data.Target.GetAvatarActor()->GetName());
				}
			}
			else
			{
				// Remove Dead tag if health is above 0
				if (ASC && ASC->HasMatchingGameplayTag(DeadTag))
				{
					ASC->RemoveLooseGameplayTag(DeadTag);
					UE_LOG(LogPristonTaleRework, Log, TEXT("%s reviveu!"), *Data.Target.GetAvatarActor()->GetName());
				}					
			}
		}
	}
	
	if (Data.EvaluatedData.Attribute == GetIncomingStatBonusAttribute())
	{
		float BonusValue = GetIncomingStatBonus();
		float Multiplier = GetStatMultiplier();
        
		SetMinPowerAttack(GetMinPowerAttack() + (BonusValue * Multiplier));
        
		SetIncomingStatBonus(0.0f);
		SetStatMultiplier(1.0f);
	}
}
