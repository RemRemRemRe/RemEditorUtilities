# Features
Provide the ability to customize any given type of `FObjectPropertyBase` properties in an `UObject` (it can be an **instanced** object)
- (**instanced**) UObject container is partially supported, including: `TArray`, `TSet`
- container properties in an UObject is fully supported, including: `TArray`, `TSet`, `TMap`
- struct property is ignored, it is better and should be able to use existing `FPropertyEditorModule::RegisterCustomPropertyTypeLayout` to do the customization

For more information, @see `FComponentBasedWidgetDetails::CustomizeDetails`
