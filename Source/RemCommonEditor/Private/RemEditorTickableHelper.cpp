// Copyright RemRemRemRe. 2025. All Rights Reserved.


#include "RemEditorTickableHelper.h"

#include "RemEditorOnlyTickableActor.h"
#include "HAL/IConsoleManager.h"
#include "Macro/RemAssertionMacros.h"


namespace 
{
    TAutoConsoleVariable CVarEnableEditorTickable(TEXT("Rem.Editor.TickableHelper.Enable"), true,
        TEXT(""));
}

FRemEditorTickableHelper::FRemEditorTickableHelper(ARemEditorOnlyTickableActor* Actor)
{
    WeakActor = Actor;
}

void FRemEditorTickableHelper::Tick(const float DeltaTime)
{
    auto* Actor = WeakActor.Get();
    RemCheckVariable(Actor, return, REM_NO_LOG_BUT_ENSURE);

    Actor->EditorTick(DeltaTime);
}

bool FRemEditorTickableHelper::IsTickable() const
{
    return CVarEnableEditorTickable.GetValueOnGameThread() && WeakActor.IsValid();
}

TStatId FRemEditorTickableHelper::GetStatId() const
{
    RETURN_QUICK_DECLARE_CYCLE_STAT(FRemEditorTickableHelper, STATGROUP_Tickables);
}
