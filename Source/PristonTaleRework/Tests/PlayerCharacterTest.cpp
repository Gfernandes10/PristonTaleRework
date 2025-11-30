#include "Misc/AutomationTest.h"
#include "PlayerCharacter.h"
#include "AbilitySystemComponent.h"
#include "GAS/AttributesSets/StatsAttributeSet.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameplayEffect.h"


// FREQ-001: The Core Gameplay Module shall allow players to move their character in a top-down third-person view.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPlayerMovementTest,
    "PristonTaleRework.CoreGameplay.Movement",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)
bool FPlayerMovementTest::RunTest(const FString& Parameters)
{
    APlayerCharacter* Player = NewObject<APlayerCharacter>();
    
    TestNotNull(TEXT("Player should exist"), Player);
    TestNotNull(TEXT("Movement component should exist"), Player->GetCharacterMovement());
    
    // Verifica se está configurado como top-down
    TestFalse(TEXT("Should not use controller rotation"), Player->bUseControllerRotationYaw);
    TestTrue(TEXT("Should orient rotation to movement"), 
        Player->GetCharacterMovement()->bOrientRotationToMovement);
    
    return true;
}

// FREQ-002: The Core Gameplay Module shall allow players to attack and cast abilities on enemies in specific combat situations using their character.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(
    FPlayerCombatTest,
    "PristonTaleRework.CoreGameplay.Combat",
    EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
)

bool FPlayerCombatTest::RunTest(const FString& Parameters)
{
    // 1. Carrega a Blueprint do player
    FString BlueprintPath = TEXT("/Game/01_PristonRework/Blueprints/BP_PTRPlayer.BP_PTRPlayer_C");
    UClass* PlayerBlueprintClass = LoadClass<APlayerCharacter>(
        nullptr,
        *BlueprintPath
    );

    if (!PlayerBlueprintClass)
    {
        AddError(FString::Printf(
            TEXT("Failed to load Blueprint: %s"), *BlueprintPath));
        return false;
    }

    // 2. Cria o World
    UWorld* TestWorld = UWorld::CreateWorld(EWorldType::Game, false);
    FWorldContext& WorldContext = GEngine->CreateNewWorldContext(EWorldType::Game);
    WorldContext.SetCurrentWorld(TestWorld);
    if (!TestWorld)
    {
        AddError(TEXT("Failed to create test world"));
        return false;
    }

    TestWorld->InitializeActorsForPlay(FURL());
    TestWorld->BeginPlay();

    // 3. Spawna usando a Blueprint 
    FActorSpawnParameters SpawnParams;
    SpawnParams.bNoFail = true;
    
    APlayerCharacter* PlayerCharacter = TestWorld->SpawnActor<APlayerCharacter>(
        PlayerBlueprintClass, 
        FVector::ZeroVector, 
        FRotator::ZeroRotator, 
        SpawnParams
    );

    TestNotNull("Player Character should be spawned", PlayerCharacter);
    
    if (PlayerCharacter)
    {
        UAbilitySystemComponent* ASC = PlayerCharacter->GetAbilitySystemComponent();
        TestNotNull("ASC should exist", ASC);
        
        if (ASC)
        {
            // Inicializar o ASC manualmente se necessário
            ASC->InitAbilityActorInfo(PlayerCharacter, PlayerCharacter);
            
            // Aguardar um frame para garantir que BeginPlay foi executado
            TestWorld->Tick(LEVELTICK_All, 0.016f);
            
            // Verificar abilities
            TArray<FGameplayAbilitySpec> Abilities = ASC->GetActivatableAbilities();
            TestTrue("Should have abilities from Blueprint", Abilities.Num() > 0);
            
            // Verificar tag de combate
            FGameplayTag PlayerTag = FGameplayTag::RequestGameplayTag(FName("Combat.CanAttack.Player"));
            TestTrue("Should have player combat tag", ASC->HasMatchingGameplayTag(PlayerTag));
        }
    }

    // Cleanup
    if (TestWorld)
    {
        GEngine->DestroyWorldContext(TestWorld);
        TestWorld->DestroyWorld(false);
    }
    
    return true;
}

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