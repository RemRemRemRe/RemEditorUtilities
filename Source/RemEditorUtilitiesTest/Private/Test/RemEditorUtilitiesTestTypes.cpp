// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "Test/RemEditorUtilitiesTestTypes.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(RemEditorUtilitiesTestTypes)

namespace Rem::Editor::Private
{
void ARemTestEditorTickableActor::EditorTick(const float DeltaSeconds)
{
    ++EditorTickCount;
    LastDeltaSeconds = DeltaSeconds;
}
}
