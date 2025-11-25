#include "Misc/AutomationTest.h"
#include "PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributesSets/StatsAttributeSet.h"
#include "GameplayEffect.h"

// Teste 1: Verificar criação básica do PlayerCharacter
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPlayerCharacterBasicTest,
    "PristonTaleRework.System.PlayerCharacter.Basic",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter  // ← Usar EditorContext
)
bool FPlayerCharacterBasicTest::RunTest(const FString& Parameters)
{
    APlayerCharacter* Player = NewObject<APlayerCharacter>();

    TestNotNull(TEXT("PlayerCharacter should be created"), Player);
    TestNotNull(TEXT("AbilitySystemComponent should exist"), Player->GetAbilitySystemComponent());
    TestNotNull(TEXT("StatsAttributeSet should exist"), Player->StatsAttributeSet);

    return true;
}

// Teste 2: Verificar stat points
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPlayerCharacterStatPointsTest,
    "PristonTaleRework.System.PlayerCharacter.StatPoints",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlayerCharacterStatPointsTest::RunTest(const FString& Parameters)
{
    APlayerCharacter* Player = NewObject<APlayerCharacter>();

    float InitialPoints = Player->GetAvailableStatPoints();
    TestTrue(TEXT("Should have initial stat points"), InitialPoints >= 0);

    return true;
}

// Teste 3: Verificar level up
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPlayerCharacterLevelUpTest,
    "PristonTaleRework.System.PlayerCharacter.LevelUp",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlayerCharacterLevelUpTest::RunTest(const FString& Parameters)
{
    APlayerCharacter* Player = NewObject<APlayerCharacter>();

    if (!Player->GetAbilitySystemComponent() || !Player->StatsAttributeSet)
    {
        AddWarning(TEXT("Cannot test level up without ASC and AttributeSet"));
        return true;
    }

    float LevelBefore = Player->GetAbilitySystemComponent()->GetNumericAttribute(
        UStatsAttributeSet::GetLevelAttribute()
    );

    Player->LevelUp();

    float LevelAfter = Player->GetAbilitySystemComponent()->GetNumericAttribute(
        UStatsAttributeSet::GetLevelAttribute()
    );

    TestEqual(TEXT("Level should increase by 1"), LevelAfter, LevelBefore + 1.0f);

    return true;
}

// Teste 4: Verificar unlock de abilities
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPlayerCharacterAbilityUnlockTest,
    "PristonTaleRework.System.PlayerCharacter.AbilityUnlock",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlayerCharacterAbilityUnlockTest::RunTest(const FString& Parameters)
{
    APlayerCharacter* Player = NewObject<APlayerCharacter>();

    FGameplayTag TestTag = FGameplayTag::RequestGameplayTag(FName("Ability.Test"));

    TestFalse(TEXT("Ability should not be unlocked initially"), Player->HasUnlockedAbilityTag(TestTag));

    Player->AddUnlockedAbility(1, TestTag, nullptr);

    TestTrue(TEXT("Ability should be unlocked after adding"), Player->HasUnlockedAbilityTag(TestTag));

    Player->RemoveUnlockedAbilityTag(TestTag);

    TestFalse(TEXT("Ability should be removed"), Player->HasUnlockedAbilityTag(TestTag));

    return true;
}