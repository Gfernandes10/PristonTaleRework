#pragma once

#include "CoreMinimal.h"
#include "EGameSaveSlots.generated.h"

UENUM(BlueprintType)
enum class EGameSaveSlots : uint8
{
	SlotA UMETA(DisplayName = "Game Slot A"),
	SlotB UMETA(DisplayName = "Game Slot B"),
	SlotC UMETA(DisplayName = "Game Slot C")
};
