#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ExperienceTableRow.generated.h"

USTRUCT(BlueprintType)
struct FExperienceTableRow : public FTableRowBase
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 Level = 1;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 ExperienceRequired = 0;
};
