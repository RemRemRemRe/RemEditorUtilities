
#include "DetailCustomizationUtilities.h"

namespace FDetailCustomizationUtilities
{
	
FText FDetailCustomizationUtilities::GetWidgetName(const UWidget* Widget)
{
	CheckPointer(Widget, return FText::GetEmpty());
	
	return Widget->IsGeneratedName() ? FText::FromName(Widget->GetFName()) : Widget->GetLabelText();;
}

bool IsArrayElementValid(const TSharedPtr<IPropertyHandle> ElementHandle)
{
	// whether the element has valid value
	uint32 IsElementValid;
	ElementHandle->GetNumChildren(IsElementValid);
	return IsElementValid != 0;
}
	
}
