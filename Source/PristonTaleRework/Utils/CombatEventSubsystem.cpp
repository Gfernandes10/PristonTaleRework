#include "CombatEventSubsystem.h"

void UCombatEventSubsystem::BroadcastEnemyDefeated(AActor* DefeatedEnemy, int32 ExperiencePoints)
{
	OnEnemyDefeated.Broadcast(DefeatedEnemy, ExperiencePoints);
}
