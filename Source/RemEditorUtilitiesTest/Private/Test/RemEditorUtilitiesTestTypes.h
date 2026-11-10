// Copyright RemRemRemRe. 2026. All Rights Reserved.

#pragma once

#include "Engine/DataAsset.h"
#include "GameplayTagContainer.h"
#include "RemEditorOnlyTickableActor.h"

// generated.h MUST be included BEFORE the type declarations: GENERATED_BODY()
// expands to a macro defined here; a trailing include leaves it undefined (C4430).
// UHT emits a matching forward declaration inside the namespace, so the include
// stays outside it.
#include "RemEditorUtilitiesTestTypes.generated.h"

namespace Rem::Editor::Private
{
/**
 * Test-only nested struct for property path tests.
 */
USTRUCT()
struct FRemTestEditorNested
{
    GENERATED_BODY()

    UPROPERTY()
    int32 Value{};
};

/**
 * Test-only struct whose reflected properties drive GetPropertyPath tests.
 */
USTRUCT()
struct FRemTestEditorData
{
    GENERATED_BODY()

    UPROPERTY()
    int32 Number{};

    UPROPERTY()
    FRemTestEditorNested Nested{};
};

/**
 * Test-only struct whose Tags member collides with the Rem tag array by member name and element type only.
 */
USTRUCT()
struct FRemTestTagLookalike
{
    GENERATED_BODY()

    UPROPERTY()
    TArray<FGameplayTag> Tags;
};

/**
 * Test-only abstract class for the DisallowedClassFlags filter test.
 */
UCLASS(Abstract)
class URemTestAbstractAsset : public UDataAsset
{
    GENERATED_BODY()
};

/**
 * Test-only tickable actor subclass exposing the protected EditorTick hook.
 */
UCLASS()
class ARemTestEditorTickableActor : public ARemEditorOnlyTickableActor
{
    GENERATED_BODY()

public:
    ARemTestEditorTickableActor() = default;

    int32 EditorTickCount{};

    float LastDeltaSeconds{};

protected:
    virtual void EditorTick(float DeltaSeconds) override;
};
}
