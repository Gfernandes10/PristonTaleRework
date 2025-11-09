// GA_AreaAttack.cpp
#include "GA_AreaAttack.h"
#include "AbilitySystemComponent.h"
#include "AbilitySystemGlobals.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "AIController.h"
#include "Blueprint/AIBlueprintHelperLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

UGA_AreaAttack::UGA_AreaAttack()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
    NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_AreaAttack::ActivateAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (TriggerEventData && TriggerEventData->Target)
    {
        TargetActor = const_cast<AActor*>(TriggerEventData->Target.Get());
    }
    
    if (!TargetActor.IsValid())
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (ACharacter* Character = Cast<ACharacter>(ActorInfo->AvatarActor.Get()))
    {
        if (AController* Controller = Character->GetController())
        {
            UAIBlueprintHelperLibrary::SimpleMoveToActor(Controller, TargetActor.Get());
        }
    }

    OnMovingToTarget(TargetActor.Get());

    // Iniciar timer para verificar distância
    GetWorld()->GetTimerManager().SetTimer(
        MovementCheckTimer,
        this,
        &UGA_AreaAttack::CheckDistanceAndAttack,
        MovementCheckInterval,
        true
    );
}
void UGA_AreaAttack::CheckDistanceAndAttack()
{
    if (!TargetActor.IsValid())
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!Avatar)
    {
        CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
        return;
    }

    float Distance = FVector::Dist(Avatar->GetActorLocation(), TargetActor->GetActorLocation());

    if (Distance <= AttackRange)
    {
        // Parar movimento
        if (ACharacter* Character = Cast<ACharacter>(Avatar))
        {
            if (AController* Controller = Character->GetController())
            {
                Controller->StopMovement();
            }
        }

        // Parar timer
        GetWorld()->GetTimerManager().ClearTimer(MovementCheckTimer);

        // Executar ataque
        ExecuteAttack();
    }
}
void UGA_AreaAttack::ExecuteAttack()
{
    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!Avatar || !TargetActor.IsValid())
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
        return;
    }

    // Rotacionar personagem para o target
    FVector Direction = TargetActor->GetActorLocation() - Avatar->GetActorLocation();
    Direction.Z = 0;
    FRotator NewRotation = Direction.Rotation();
    Avatar->SetActorRotation(NewRotation);

    // Aplicar dano em área usando a localização do target como origem
    FVector OriginLocation = TargetActor->GetActorLocation();
    ApplyDamageToEnemiesInArea(OriginLocation);

    OnAttackExecuted();

    // Tocar animação
    if (AttackMontage)
    {
        UE_LOG(LogTemp, Warning, TEXT("Iniciando montage: %s"), *AttackMontage->GetName());
        
        UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
            this,
            NAME_None,
            AttackMontage,
            1.0f,
            NAME_None,
            false,
            0.0f,
            0.0f,
            true
        );

        if (Task)
        {
            UE_LOG(LogTemp, Warning, TEXT("Task criada com sucesso"));
            
            Task->OnCompleted.AddDynamic(this, &UGA_AreaAttack::OnMontageCompleted);
            Task->OnInterrupted.AddDynamic(this, &UGA_AreaAttack::OnMontageInterrupted);
            Task->OnCancelled.AddDynamic(this, &UGA_AreaAttack::OnMontageInterrupted);
            Task->ReadyForActivation();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Falha ao criar Task"));
            
            EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
        }
    }
    else
    {
        EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
    }
}

void UGA_AreaAttack::EndAbility(
    const FGameplayAbilitySpecHandle Handle,
    const FGameplayAbilityActorInfo* ActorInfo,
    const FGameplayAbilityActivationInfo ActivationInfo,
    bool bReplicateEndAbility,
    bool bWasCancelled)
{
    GetWorld()->GetTimerManager().ClearTimer(MovementCheckTimer);
    TargetActor.Reset();

    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}

void UGA_AreaAttack::ApplyDamageToEnemiesInArea(const FVector& OriginLocation)
{
    TArray<AActor*> Enemies = FindEnemiesInArea(OriginLocation);

    if (!DamageEffect)
    {
        UE_LOG(LogTemp, Warning, TEXT("GA_AreaAttack: DamageEffect não configurado!"));
        return;
    }

    UAbilitySystemComponent* SourceASC = GetAbilitySystemComponentFromActorInfo();
    if (!SourceASC)
        return;

    for (AActor* Enemy : Enemies)
    {
        UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Enemy);
        if (!TargetASC)
            continue;

        FGameplayEffectContextHandle EffectContext = SourceASC->MakeEffectContext();
        EffectContext.AddSourceObject(this);

        FGameplayEffectSpecHandle SpecHandle = SourceASC->MakeOutgoingSpec(
            DamageEffect,
            GetAbilityLevel(),
            EffectContext
        );

        if (SpecHandle.IsValid())
        {
            const FGameplayTag MultTag = FGameplayTag::RequestGameplayTag(FName("Combat.DamageMultiplier"));
            if (FGameplayEffectSpec* Spec = SpecHandle.Data.Get())
            {
                Spec->SetSetByCallerMagnitude(MultTag, DamageMultiplier);
                SourceASC->ApplyGameplayEffectSpecToTarget(*Spec, TargetASC);
            }
        }
    }

    UE_LOG(LogTemp, Log, TEXT("GA_AreaAttack: %d inimigos atingidos"), Enemies.Num());
}

TArray<AActor*> UGA_AreaAttack::FindEnemiesInArea(const FVector& OriginLocation)
{
    TArray<AActor*> Result;
    TArray<FHitResult> HitResults;

    AActor* Avatar = GetAvatarActorFromActorInfo();
    if (!Avatar)
        return Result;

    FCollisionQueryParams QueryParams;
    QueryParams.AddIgnoredActor(Avatar);
    
    TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
    ObjectTypes.Add(UEngineTypes::ConvertToObjectType(ECC_WorldDynamic));

    bool bHit = GetWorld()->SweepMultiByObjectType(
        HitResults,
        OriginLocation,
        OriginLocation,
        FQuat::Identity,
        ObjectTypes,
        FCollisionShape::MakeSphere(AreaRadius),
        QueryParams
    );

    if (bHit)
    {
        FVector ForwardVector = Avatar->GetActorForwardVector();
        static const FGameplayTag CombatCanAttackEnemyTag = FGameplayTag::RequestGameplayTag(FName("Combat.CanAttack.Enemy"));


        for (const FHitResult& Hit : HitResults)
        {
            if (!Hit.GetActor() || Hit.GetActor() == Avatar)
                continue;

            UAbilitySystemComponent* TargetASC = UAbilitySystemGlobals::GetAbilitySystemComponentFromActor(Hit.GetActor());
            if (!TargetASC || !TargetASC->HasMatchingGameplayTag(CombatCanAttackEnemyTag))
                continue;

            // Se for cone, verificar ângulo
            if (AreaShape == EAreaShape::Cone)
            {
                if (!IsInCone(OriginLocation, ForwardVector, Hit.GetActor()->GetActorLocation()))
                    continue;
            }

            Result.AddUnique(Hit.GetActor());
        }
    }

    return Result;
}

bool UGA_AreaAttack::IsInCone(const FVector& OriginLocation, const FVector& ForwardVector, const FVector& TargetLocation) const
{
    FVector ToTarget = (TargetLocation - OriginLocation).GetSafeNormal();
    float DotProduct = FVector::DotProduct(ForwardVector, ToTarget);
    float AngleRad = FMath::Acos(DotProduct);
    float AngleDeg = FMath::RadiansToDegrees(AngleRad);

    return AngleDeg <= (ConeAngle / 2.0f);
}

void UGA_AreaAttack::OnMontageCompleted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_AreaAttack::OnMontageInterrupted()
{
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}
void UGA_AreaAttack::OnMovingToTarget(AActor* Target)
{
    // Implementação Blueprint (opcional)
}

void UGA_AreaAttack::OnAttackExecuted()
{
    // Implementação Blueprint (opcional)
}