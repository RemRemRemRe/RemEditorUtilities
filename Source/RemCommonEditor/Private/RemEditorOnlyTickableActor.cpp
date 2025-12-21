// Copyright RemRemRemRe. 2025. All Rights Reserved.


#include "RemEditorOnlyTickableActor.h"

#include "RemEditorTickableHelper.h"
#include "TimerManager.h"
#include "Engine/World.h"
#include "Macro/RemAssertionMacros.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RemEditorOnlyTickableActor)

namespace 
{
    TAutoConsoleVariable CVarEnableEditorTickableActor(TEXT("Rem.Editor.Tickable.Actor.Enable"), true,
    TEXT(""));
}

ARemEditorOnlyTickableActor::ARemEditorOnlyTickableActor()
{
    PrimaryActorTick.bCanEverTick = true;
    // PrimaryActorTick.bStartWithTickEnabled = true; // set on parent
}

void ARemEditorOnlyTickableActor::PostActorCreated()
{
    Super::PostActorCreated();

    RemCheckCondition(CVarEnableEditorTickableActor.GetValueOnGameThread(), return;, REM_NO_LOG_OR_ASSERTION);
    
    auto* World = GetWorld();
    RemCheckVariable(World, return;);
    
    if (!World->IsEditorWorld())
    {
        return;
    }
    
    // EditorTickableHelper = MakeShared<FRemEditorTickableHelper>(this);

    if (!PrimaryActorTick.bStartWithTickEnabled)
    {
        return;
    }
    
    World->GetTimerManager().SetTimer(EditorTickHandle,
        FTimerDelegate::CreateUObject(this, &ThisClass::EditorTick, PrimaryActorTick.TickInterval),
        FMath::Max(PrimaryActorTick.TickInterval, UE_KINDA_SMALL_NUMBER),
        {.bLoop = true, .bMaxOncePerFrame = true});
    
    RemCheckVariable(EditorTickHandle);
}

void ARemEditorOnlyTickableActor::Destroyed()
{
    auto* World = GetWorld();
    RemCheckVariable(World, return;);
    
    World->GetTimerManager().ClearTimer(EditorTickHandle);
    
    Super::Destroyed();
}

void ARemEditorOnlyTickableActor::EditorTick(const float DeltaSeconds)
{
    auto* World = GetWorld();
    RemCheckVariable(World, return;);
    
    RemCheckCondition(CVarEnableEditorTickableActor.GetValueOnGameThread(), return;, REM_NO_LOG_OR_ASSERTION);
    
    BP_EditorTick(FMath::Max(World->GetDeltaSeconds(), DeltaSeconds));
}
