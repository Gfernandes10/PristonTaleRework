// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BaseCharacter.h"
#include "EnemyCharacter.generated.h"

/**
 * 
 */
UCLASS()
class PRISTONTALEREWORK_API AEnemyCharacter : public ABaseCharacter
{
	GENERATED_BODY()

	AEnemyCharacter();

	virtual void BeginPlay() override;


	
};
