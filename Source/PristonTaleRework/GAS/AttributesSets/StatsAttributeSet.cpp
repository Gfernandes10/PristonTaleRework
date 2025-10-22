// Fill out your copyright notice in the Description page of Project Settings.


#include "StatsAttributeSet.h"
#include "Net/UnrealNetwork.h"
#include "GameplayEffectExtension.h"
#include "AbilitySystemComponent.h"
#include "PlayerCharacter.h"

UStatsAttributeSet::UStatsAttributeSet()
{
	Strength = 1.f;
	Intelligence = 1.f;
	Vitality = 1.f;
	Agility = 1.f;
	AvailableStatPoints = 0.f;
	Level = 1.f;
}

void UStatsAttributeSet::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME_CONDITION_NOTIFY(UStatsAttributeSet, Strength, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStatsAttributeSet, Intelligence, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStatsAttributeSet, Vitality, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStatsAttributeSet, Agility, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStatsAttributeSet, AvailableStatPoints, COND_None, REPNOTIFY_Always);
	DOREPLIFETIME_CONDITION_NOTIFY(UStatsAttributeSet, Level, COND_None, REPNOTIFY_Always);
}

void UStatsAttributeSet::PostGameplayEffectExecute(const FGameplayEffectModCallbackData& Data)
{
	Super::PostGameplayEffectExecute(Data);

	if (!Data.Target.AbilityActorInfo.IsValid()) return;

	const FGameplayAttribute& Attr = Data.EvaluatedData.Attribute;
	UAbilitySystemComponent* ASC = Data.Target.AbilityActorInfo->AbilitySystemComponent.Get();
	if (!ASC) return;
	
	if (Attr == GetVitalityAttribute())
	{
		AActor* OwnerActor = Data.Target.AbilityActorInfo->OwnerActor.Get();
		if (!OwnerActor) return;

		APlayerCharacter* Player = Cast<APlayerCharacter>(OwnerActor);
		// if (Player)
		// {
		// 	Player->UpdateBasicAttributesBaseOnStats();
		// }
	}
}