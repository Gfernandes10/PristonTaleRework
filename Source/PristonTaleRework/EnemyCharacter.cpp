// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"

AEnemyCharacter::AEnemyCharacter()
{
	
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (AbilitySystemComponent)
	{
		static const FGameplayTag CombatCanAttackEnemyTag = FGameplayTag::RequestGameplayTag(FName("Combat.CanAttack.Enemy"));
		
		AbilitySystemComponent->AddLooseGameplayTag(CombatCanAttackEnemyTag);
		
	}
}
