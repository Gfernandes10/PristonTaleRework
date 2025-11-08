#include "GA_BuffAbility.h"
#include "AbilitySystemComponent.h"
#include "Abilities/Tasks/AbilityTask_PlayMontageAndWait.h"
#include "GameFramework/Character.h"
#include "Kismet/GameplayStatics.h"

UGA_BuffAbility::UGA_BuffAbility()
{
    InstancingPolicy = EGameplayAbilityInstancingPolicy::InstancedPerActor;
NetExecutionPolicy = EGameplayAbilityNetExecutionPolicy::LocalPredicted;
}

void UGA_BuffAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                     const FGameplayAbilityActorInfo* ActorInfo,
                                     const FGameplayAbilityActivationInfo ActivationInfo,
                                     const FGameplayEventData* TriggerEventData)
{
    if (!CommitAbility(Handle, ActorInfo, ActivationInfo))
    {
        EndAbility(Handle, ActorInfo, ActivationInfo, true, true);
        return;
    }

    if (BuffMontage)
    {
        AActor* Avatar = GetAvatarActorFromActorInfo();
        ACharacter* Character = Cast<ACharacter>(Avatar);

        if (Character)
        {
            if (AController* Controller = Character->GetController())
            {
                Controller->StopMovement();
            }
        }
        UAnimInstance* AnimInstance = Character ? Character->GetMesh()->GetAnimInstance() : nullptr;

        UE_LOG(LogTemp, Warning, TEXT("🎬 BuffMontage: %s"), *BuffMontage->GetName());
        UE_LOG(LogTemp, Warning, TEXT("🎭 AnimInstance: %s"), AnimInstance ? TEXT("VÁLIDO") : TEXT("NULO"));

        UAbilityTask_PlayMontageAndWait* Task = UAbilityTask_PlayMontageAndWait::CreatePlayMontageAndWaitProxy(
        this, NAME_None, BuffMontage,
        1.0f,
        NAME_None,
        false,
        0.0f,
        0.0f,
        true
        );

        if (Task)
        {
            UE_LOG(LogTemp, Warning, TEXT("✅ Task criada com sucesso"));
            Task->OnCompleted.AddDynamic(this, &UGA_BuffAbility::OnMontageCompleted);
            Task->OnInterrupted.AddDynamic(this, &UGA_BuffAbility::OnMontageInterrupted);
            Task->OnCancelled.AddDynamic(this, &UGA_BuffAbility::OnMontageInterrupted);
            Task->ReadyForActivation();
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("❌ Falha ao criar Task"));
            EndAbility(Handle, CurrentActorInfo, CurrentActivationInfo, true, false);
        }
    }
    else
    {
        ApplyBuffEffect();
        EndAbility(Handle, ActorInfo, ActivationInfo, true, false);
    }
}

void UGA_BuffAbility::OnMontageCompleted()
{
    UE_LOG(LogTemp, Warning, TEXT("🎯 OnMontageCompleted chamado!"));
    ApplyBuffEffect();
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, false);
}

void UGA_BuffAbility::OnMontageInterrupted()
{
    UE_LOG(LogTemp, Warning, TEXT("⚠️ OnMontageInterrupted chamado!"));
    EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
}


void UGA_BuffAbility::ApplyBuffEffect()
{
    if (!BuffEffect)
        return;

    UAbilitySystemComponent* ASC = GetAbilitySystemComponentFromActorInfo();
    if (!ASC)
        return;

    FGameplayEffectContextHandle EffectContext = ASC->MakeEffectContext();
    EffectContext.AddSourceObject(GetAvatarActorFromActorInfo());

    FGameplayEffectSpecHandle SpecHandle = ASC->MakeOutgoingSpec(BuffEffect, GetAbilityLevel(), EffectContext);

    if (SpecHandle.IsValid())
    {
        ASC->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data.Get());

        if (BuffParticle)
        {
            UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), BuffParticle, 
                GetAvatarActorFromActorInfo()->GetActorLocation());
        }

        if (BuffSound)
        {
            UGameplayStatics::PlaySoundAtLocation(GetWorld(), BuffSound, 
                GetAvatarActorFromActorInfo()->GetActorLocation());
        }

        OnBuffApplied();
    }
}

void UGA_BuffAbility::EndAbility(const FGameplayAbilitySpecHandle Handle,
                                const FGameplayAbilityActorInfo* ActorInfo,
                                const FGameplayAbilityActivationInfo ActivationInfo,
                                bool bReplicateEndAbility,
                                bool bWasCancelled)
{
    Super::EndAbility(Handle, ActorInfo, ActivationInfo, bReplicateEndAbility, bWasCancelled);
}
