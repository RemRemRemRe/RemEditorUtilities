// Copyright RemRemRemRe. 2025. All Rights Reserved.

#pragma once

#include "GameFramework/Actor.h"

#include "RemEditorOnlyTickableActor.generated.h"

struct FRemEditorTickableHelper;

UCLASS()
class REMCOMMONEDITOR_API ARemEditorOnlyTickableActor : public AActor
{
    GENERATED_BODY()

    // TSharedPtr<FRemEditorTickableHelper> EditorTickableHelper;
    FTimerHandle EditorTickHandle{};
protected:
    
    ARemEditorOnlyTickableActor();
    
    friend FRemEditorTickableHelper;
    
    virtual void PostActorCreated() override;
    virtual void Destroyed() override;
    
    virtual void EditorTick(float DeltaSeconds);
    
    UFUNCTION(BlueprintImplementableEvent, DisplayName = "EditorTick", Category = RemEditorOnlyTickableActor, CallInEditor)
    void BP_EditorTick(float DeltaSeconds);
    
};
