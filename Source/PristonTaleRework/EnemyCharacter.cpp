// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyCharacter.h"
#include "AIController.h"
#include "AbilitySystemComponent.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "Kismet/GameplayStatics.h"


AEnemyCharacter::AEnemyCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	// Configurar tag padrão de ataque
	AttackAbilityTag = FGameplayTag::RequestGameplayTag("Ability.Attack.Melee");

	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;
}

void AEnemyCharacter::BeginPlay()
{
	Super::BeginPlay();
	if (AbilitySystemComponent)
	{
		static const FGameplayTag CombatCanAttackEnemyTag = FGameplayTag::RequestGameplayTag(FName("Combat.CanAttack.Enemy"));
		
		AbilitySystemComponent->AddLooseGameplayTag(CombatCanAttackEnemyTag);

		FGameplayTag AttackActiveTag = FGameplayTag::RequestGameplayTag("Ability.Attack.Active");
        
		AbilitySystemComponent->RegisterGameplayTagEvent(AttackActiveTag)
			.AddUObject(this, &AEnemyCharacter::OnAttackTagChanged);
		
	}
	// GetWorld()->GetTimerManager().SetTimer(
	// 	AttackCheckTimer,
	// 	this,
	// 	&AEnemyCharacter::CheckPlayerDistance,
	// 	AttackCheckInterval,
	// 	true
	// );
	
	
}
void AEnemyCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}
void AEnemyCharacter::StartFollowingPlayer(AActor* Player)
{
	if (!Player)
	{
		UE_LOG(LogTemp, Warning, TEXT("EnemyCharacter: Player is null"));
		return;
	}

	TargetPlayer = Player;

	// Iniciar timer para verificar distância
	GetWorld()->GetTimerManager().SetTimer(
		AttackCheckTimer,
		this,
		&AEnemyCharacter::CheckPlayerDistance,
		AttackCheckInterval,
		true
	);

	UE_LOG(LogTemp, Log, TEXT("EnemyCharacter: Started following player"));
}
void AEnemyCharacter::StopFollowingPlayer()
{
	if (AAIController* AIController = Cast<AAIController>(GetController()))
	{
		AIController->StopMovement();
	}

	// Limpar timer de ataque
	GetWorld()->GetTimerManager().ClearTimer(AttackCheckTimer);
    
	// Limpar referência ao jogador
	TargetPlayer.Reset();

	UE_LOG(LogTemp, Log, TEXT("EnemyCharacter: Stopped following player"));
}

void AEnemyCharacter::CheckPlayerDistance()
{
	AAIController* AICtrl = Cast<AAIController>(GetController());

	if (!AICtrl)
	{
		UE_LOG(LogTemp, Error, TEXT("EnemyCharacter %s: NO AI CONTROLLER!"), *GetName());
		return;
	}
	
	if (!TargetPlayer.IsValid())
	{
		AActor* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
		if (!Player)
		{
			return;
		}
		TargetPlayer = Player;
	}
	
	AActor* Player = TargetPlayer.Get();
	float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
	

	// Se estiver fora do alcance de detecção, parar
	if (Distance > DetectionRange)
	{
		AICtrl->StopMovement();
		return;
	}

	if (Distance <= AttackRange)
	{
		// Parar movimento
		AICtrl->StopMovement();

		// Tentar atacar
		if (CanAttack())
		{
			ExecuteAttack();
		}
	}
	else
	{
		// Continuar seguindo o player
		if (AController* Ctrl = GetController())
		{
			UAIBlueprintHelperLibrary::SimpleMoveToActor(Ctrl, Player);
		}
	}
}

void AEnemyCharacter::ExecuteAttack()
{
	if (!TargetPlayer.IsValid() || !AbilitySystemComponent)
	{
		return;
	}

	// Rotacionar para o player
	FVector Direction = TargetPlayer->GetActorLocation() - GetActorLocation();
	Direction.Z = 0;
	FRotator NewRotation = Direction.Rotation();
	SetActorRotation(NewRotation);

	// Criar payload com o target
	FGameplayEventData Payload;
	Payload.Target = TargetPlayer.Get();

	// Ativar habilidade de ataque
	AbilitySystemComponent->HandleGameplayEvent(AttackAbilityTag, &Payload);

	//UE_LOG(LogTemp, Log, TEXT("EnemyCharacter: Executed attack on player"));
}
void AEnemyCharacter::OnAttackTagChanged(const FGameplayTag Tag, int32 NewCount)
{
	if (NewCount == 0) // Tag foi removida = ataque terminou
	{
		OnAttackFinished.Broadcast();
	}
}
bool AEnemyCharacter::CanAttack()
{
	if (!AbilitySystemComponent)
	{
		return false;
	}

	// Verificar se está em cooldown
	FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag("Cooldown.Attack.Melee");
	if (AbilitySystemComponent->HasMatchingGameplayTag(CooldownTag))
	{
		return false;
	}

	// Verificar se está morto
	FGameplayTag DeadTag = FGameplayTag::RequestGameplayTag("Character.State.Dead");
	if (AbilitySystemComponent->HasMatchingGameplayTag(DeadTag))
	{
		return false;
	}
	
	if (!GetPlayerTarget())
	{
		return false;
	};
	
	return true;
}
bool AEnemyCharacter::GetPlayerTarget()
{
	// Se já temos um target válido, retornar
	if (TargetPlayer.IsValid())
	{
		return true;
	}

	// Caso contrário, buscar o player
	AActor* Player = UGameplayStatics::GetPlayerCharacter(GetWorld(), 0);
	if (Player)
	{
		TargetPlayer = Player;
		UE_LOG(LogTemp, Log, TEXT("Enemy %s: Player target cached"), *GetName());
		return true;
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("Enemy %s: Player not found"), *GetName());
		return false;
	}
}