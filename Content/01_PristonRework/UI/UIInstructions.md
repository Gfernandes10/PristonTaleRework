# UI Development Guide - Priston Tale Rework

## Overview
This document provides all necessary information for implementing the UI system for Priston Tale Rework. The game uses Unreal Engine's Gameplay Ability System (GAS) for attributes, stats, and combat mechanics.

---

## 1. Character Stats System

### 1.1 Primary Stats (StatsAttributeSet)
These are the core character stats that players can manually increase:

```cpp
// Accessing from APlayerCharacter
APlayerCharacter* Player = // Get player reference
UStatsAttributeSet* Stats = Player->StatsAttributeSet;

// Available Stats:
float Strength = Stats->GetStrength();
float Intelligence = Stats->GetIntelligence();
float Vitality = Stats->GetVitality();
float Agility = Stats->GetAgility();
float Level = Stats->GetLevel();
float AvailablePoints = Stats->GetAvailableStatPoints();
```

**Blueprint Access:**
- Use `Get Ability System Component` → `Get Numeric Attribute` with attribute names like `StatsAttributeSet.Strength`

### 1.2 Adding Stat Points
Players can allocate stat points using:

```cpp
// C++
Player->AddStatPoint(StatTag, Amount);

// Example tags:
FGameplayTag::RequestGameplayTag("Data.Stats.Strength")
FGameplayTag::RequestGameplayTag("Data.Stats.Intelligence")
FGameplayTag::RequestGameplayTag("Data.Stats.Vitality")
FGameplayTag::RequestGameplayTag("Data.Stats.Agility")
```

**Blueprint:**
```
Call Function: Add Stat Point
- Stat Tag: (Select from dropdown)
- Amount: 1 (or desired value)
```

---

## 2. Basic Attributes (BasicAttributeSet)

These are calculated attributes based on primary stats:

### 2.1 Health & Mana
```cpp
UBasicAttributeSet* BasicAttrs = Player->BasicAttributeSet;

float CurrentHealth = BasicAttrs->GetHealth();
float MaxHealth = BasicAttrs->GetMaxHealth();
float CurrentMana = BasicAttrs->GetMana();
float MaxMana = BasicAttrs->GetMaxMana();

// For UI bars (0.0 to 1.0):
float HealthPercent = CurrentHealth / MaxHealth;
float ManaPercent = CurrentMana / MaxMana;
```

**Important Tags:**
- `Character.State.Regen.Health` - Active when health regeneration is happening
- `Character.State.Regen.Mana` - Active when mana regeneration is happening
- `Character.State.Dead` - Active when character is dead

### 2.2 Combat Stats
```cpp
float MinAttack = BasicAttrs->GetMinPowerAttack();
float MaxAttack = BasicAttrs->GetMaxPowerAttack();
float Defense = BasicAttrs->GetDefense();
float DefenseRate = BasicAttrs->GetDefenseRate(); // 0.0 to 0.7 (0% to 70%)
```

---

## 3. Experience & Leveling System

### 3.1 Experience Display
```cpp
int32 CurrentXP = Player->CurrentExperience;
int32 RequiredXP = Player->GetExperienceForNextLevel();
float Progress = Player->GetExperienceProgress(); // Returns 0.0 to 1.0

// For XP bar:
// Fill Amount = Progress
// Text = CurrentXP / RequiredXP
```

### 3.2 Leveling Up
Level up happens automatically when sufficient XP is gained. Listen for attribute changes on `StatsAttributeSet.Level`.

---

## 4. Combat System UI

### 4.1 Damage Numbers
Listen for changes in `BasicAttributeSet.Health` to display damage/healing numbers.

```cpp
// In your UI widget
void OnHealthChanged(const FOnAttributeChangeData& Data)
{
    float DamageTaken = Data.OldValue - Data.NewValue;
    if (DamageTaken > 0)
    {
        // Display damage number at character location
        ShowDamageNumber(DamageTaken);
    }
    else if (DamageTaken < 0)
    {
        // Display healing number
        ShowHealingNumber(FMath::Abs(DamageTaken));
    }
}
```

### 4.2 Combat Tags (for UI indicators)
```cpp
UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();

// Check combat state:
bool IsAttacking = ASC->HasMatchingGameplayTag(
    FGameplayTag::RequestGameplayTag("Ability.Attack.Melee"));
    
bool IsDead = ASC->HasMatchingGameplayTag(
    FGameplayTag::RequestGameplayTag("Character.State.Dead"));

bool CanAttack = ASC->HasMatchingGameplayTag(
    FGameplayTag::RequestGameplayTag("Combat.CanAttack.Enemy"));
```

### 4.3 Cooldowns
Check ability cooldowns for skill buttons:

```cpp
FGameplayTag CooldownTag = FGameplayTag::RequestGameplayTag("Cooldown.Attack.Melee");

if (ASC->HasMatchingGameplayTag(CooldownTag))
{
    // Get remaining cooldown time
    FGameplayEffectQuery Query;
    Query.EffectTagQuery = FGameplayTagQuery::MakeQuery_MatchAnyTags(
        FGameplayTagContainer(CooldownTag));
    
    TArray<float> Durations = ASC->GetActiveEffectsTimeRemaining(Query);
    float RemainingTime = Durations.Num() > 0 ? Durations[0] : 0.0f;
    
    // Update cooldown UI (circular fill, timer text, etc.)
}
```

---

## 5. Save/Load System UI

### 5.1 Save Slots
```cpp
enum class EGameSaveSlots : uint8
{
    SlotA,
    SlotB,
    SlotC
};

// Check if save exists:
bool bExists = Player->DoesSaveGameExist(EGameSaveSlots::SlotA);

// Save game:
Player->SaveGame(EGameSaveSlots::SlotA);

// Load game:
Player->LoadGame(EGameSaveSlots::SlotA);

// Delete save:
Player->DeleteSaveGame(EGameSaveSlots::SlotA);
```

### 5.2 Display Save Data
After loading, retrieve:
- Character stats (Strength, Intelligence, etc.)
- Current Level
- Current Experience
- Current Health/Mana
- Character position (saved, but you might not need to display it)

---

## 6. Creating UI Widgets (Blueprint Tutorial)

### 6.1 Health/Mana Bars

**Step 1:** Create Widget Blueprint
1. Content Browser → Right-click → User Interface → Widget Blueprint
2. Name it `WBP_PlayerStats`

**Step 2:** Add Progress Bars
1. Add two `Progress Bar` widgets (Health and Mana)
2. Set their colors (Red for Health, Blue for Mana)

**Step 3:** Bind Progress
1. For Health Bar → Details Panel → Percent → Bind
2. Create binding function:
```
Get Player Character → Get Ability System Component 
→ Get Gameplay Attribute Value (Attribute: BasicAttributeSet.Health)
→ Divide by Get Gameplay Attribute Value (BasicAttributeSet.MaxHealth)
```

**Step 4:** Add Text Labels
Add text widgets showing "HP: 100/100" format using same binding approach.

### 6.2 Stat Allocation Panel

**Step 1:** Create Widget
1. Create `WBP_StatAllocation`
2. Add vertical box with:
   - Text label for stat name
   - Text label for current value
   - Button to add points

**Step 2:** Button OnClick Event
```
On Button Clicked → Get Player Character 
→ Add Stat Point (Stat Tag: Data.Stats.Strength, Amount: 1)
```

**Step 3:** Update Available Points Display
```
Get Player Character → Get Available Stat Points → Set Text
```

### 6.3 Experience Bar

**Step 1:** Create Progress Bar
1. Add Progress Bar to HUD
2. Set visual style

**Step 2:** Bind Progress
```
Get Player Character → Get Experience Progress → Set Progress Bar Percent
```

**Step 3:** Add Text (XP/Required XP)
```
Get Player Character → Get Current Experience (Store as A)
Get Player Character → Get Experience For Next Level (Store as B)
Format Text: "{A} / {B}"
```

---

## 7. Listening for Attribute Changes

### 7.1 C++ Delegate Binding
```cpp
// In your UI class
void UMyHealthWidget::NativeConstruct()
{
    Super::NativeConstruct();
    
    APlayerCharacter* Player = // Get reference
    UAbilitySystemComponent* ASC = Player->GetAbilitySystemComponent();
    
    // Bind to health changes
    ASC->GetGameplayAttributeValueChangeDelegate(
        UBasicAttributeSet::GetHealthAttribute()
    ).AddUObject(this, &UMyHealthWidget::OnHealthChanged);
}

void UMyHealthWidget::OnHealthChanged(const FOnAttributeChangeData& Data)
{
    // Update health bar
    float Percent = Data.NewValue / Player->BasicAttributeSet->GetMaxHealth();
    HealthBar->SetPercent(Percent);
}
```

### 7.2 Blueprint Alternative
Use **Event Tick** or **Timer** to poll attributes every 0.1 seconds for simpler implementation (less efficient but easier).

---

## 8. Important Gameplay Tags Reference

### Combat Tags
- `Ability.Attack.Melee` - Basic melee attack
- `Ability.Attack.Area` - Area attack ability
- `Combat.CanAttack.Enemy` - Target is attackable
- `Combat.DamageMultiplier` - Used for damage scaling
- `Cooldown.Attack.Melee` - Melee attack cooldown

### Character State Tags
- `Character.State.Dead` - Character is dead
- `Character.State.Regen.Health` - Health regenerating
- `Character.State.Regen.Mana` - Mana regenerating

### Data Tags (for stat modification)
- `Data.Stats.Strength`
- `Data.Stats.Intelligence`
- `Data.Stats.Vitality`
- `Data.Stats.Agility`
- `Data.Stats.ChangeAmount`
- `Data.Stats.AvailableStatPoints`

---

## 9. Common UI Patterns

### 9.1 Floating Damage Numbers
1. Create widget with text
2. Add animation (move up + fade out)
3. Spawn at damage location
4. Play animation and destroy after completion

### 9.2 Stat Comparison Tooltip
When hovering over stat allocation buttons:
```
Current: Strength 10
+1 Strength = +5 Max Health, +2 Min Damage, +3 Max Damage
```
Calculate using the formulas from `UpdateBasicAttributesBaseOnStats()`.

### 9.3 Cooldown Visual
1. Use circular progress indicator
2. Fill from 1.0 to 0.0 as cooldown expires
3. Display remaining seconds as text overlay

---

## 10. Testing & Debugging

### Console Commands (use ~ key)
```
// Add stat points
stat.fps 1  // Show FPS
showdebug abilitysystem  // Show GAS debug info

// In C++, you can add custom commands:
Player->AddStatPoint(StrengthTag, 5);
Player->AddExperience(1000);
```

### Debug Attributes in Blueprint
Add `Print String` nodes showing attribute values during development.

---

## 11. Performance Tips

1. **Use Attribute Changed Delegates** instead of Event Tick when possible
2. **Cache Player Reference** - Store reference to player character instead of getting it every frame
3. **Pool Damage Numbers** - Reuse damage number widgets instead of spawning new ones
4. **Update Only When Visible** - Don't update hidden UI elements
5. **Batch Updates** - Update all stats in one tick instead of separately

---

## 12. Additional Resources

- Player Character Class: `APlayerCharacter`
- Stats Attribute Set: `UStatsAttributeSet`
- Basic Attributes: `UBasicAttributeSet`
- Save Game Class: `UPlayerSaveGame`

For any questions about specific systems or implementation details, refer to the source files mentioned in the code excerpts or contact the development team.

---

**Good luck with the UI implementation!** 🎮