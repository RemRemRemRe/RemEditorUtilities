// Copyright RemRemRemRe. 2025. All Rights Reserved.

#pragma once

#include "TickableEditorObject.h"

class ARemEditorOnlyTickableActor;

struct REMCOMMONEDITOR_API FRemEditorTickableHelper : FTickableEditorObject
{
    TWeakObjectPtr<ARemEditorOnlyTickableActor> WeakActor{};

    explicit FRemEditorTickableHelper(ARemEditorOnlyTickableActor* Actor);
    
protected:
    virtual void Tick(float DeltaTime) override;
    virtual bool IsTickable() const override;
    virtual TStatId GetStatId() const override;
    
};
