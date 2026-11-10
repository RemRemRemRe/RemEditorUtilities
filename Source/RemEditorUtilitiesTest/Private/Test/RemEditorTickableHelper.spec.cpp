// Copyright RemRemRemRe. 2026. All Rights Reserved.

#include "Test/RemEditorUtilitiesTestTypes.h"

#include "Engine/World.h"
#include "Misc/AutomationTest.h"
#include "RemEditorTickableHelper.h"
#include "RemTestWorld.h"

#if WITH_DEV_AUTOMATION_TESTS

namespace
{
using Rem::Editor::Private::ARemTestEditorTickableActor;
}

DEFINE_SPEC(FRemEditorTickableHelperTest, "Rem.CommonEditor.TickableHelper",
    EAutomationTestFlags_ApplicationContextMask | EAutomationTestFlags::ProductFilter);

void FRemEditorTickableHelperTest::Define()
{
    Describe(TEXT("IsTickable"), [this]
    {
        It(TEXT("should be false without a live actor"), [this]
        {
            FRemEditorTickableHelper Helper{nullptr};

            TestFalse(TEXT("not tickable"), static_cast<const FTickableEditorObject&>(Helper).IsTickable());
        });

        It(TEXT("should track the actor lifetime"), [this]
        {
            Rem::Test::FRemTestWorld World;
            auto* Actor = World->SpawnActor<ARemTestEditorTickableActor>();

            {
                FRemEditorTickableHelper Helper{Actor};
                TestTrue(TEXT("tickable while alive"), static_cast<const FTickableEditorObject&>(Helper).IsTickable());
            }

            World->DestroyActor(Actor);

            {
                FRemEditorTickableHelper Helper{Actor};
                TestFalse(TEXT("not tickable after destroy"),
                    static_cast<const FTickableEditorObject&>(Helper).IsTickable());
            }
        });
    });

    Describe(TEXT("Tick"), [this]
    {
        It(TEXT("should forward the delta time to the actor editor tick"), [this]
        {
            Rem::Test::FRemTestWorld World;
            auto* Actor = World->SpawnActor<ARemTestEditorTickableActor>();
            FRemEditorTickableHelper Helper{Actor};

            // Tick is protected in the helper; dispatch through the public base interface
            static_cast<FTickableEditorObject&>(Helper).Tick(0.5f);

            // synchronous within the test body; the editor loop cannot interleave here
            TestEqual(TEXT("ticked"), Actor->EditorTickCount, 1);
            TestEqual(TEXT("delta"), Actor->LastDeltaSeconds, 0.5f, 1e-6f);
        });
    });
}

#endif // WITH_DEV_AUTOMATION_TESTS
