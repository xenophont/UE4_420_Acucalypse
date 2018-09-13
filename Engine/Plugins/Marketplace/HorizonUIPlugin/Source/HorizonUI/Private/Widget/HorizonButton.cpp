// Created by dorgon, All Rights Reserved.
// email: dorgonman@hotmail.com
// blog: dorgon.horizon-studio.net
#include "Widget/HorizonButton.h"

void UHorizonButton::OnWidgetRebuilt()
{
	Super::OnWidgetRebuilt();
	OnPressed.AddDynamic(this, &UHorizonButton::OnPressedButton);
	OnReleased.AddDynamic(this, &UHorizonButton::OnReleasedButton);
	OnClicked.AddDynamic(this, &UHorizonButton::OnClickedButton);
	OnHovered.AddDynamic(this, &UHorizonButton::OnHoveredButton);
	OnUnhovered.AddDynamic(this, &UHorizonButton::OnUnhoveredButton);

}



void UHorizonButton::OnPressedButton()
{
	OnPressedDelegate.Broadcast(this);
	OnPressedDelegateNative.Broadcast(this);
	
}

void UHorizonButton::OnReleasedButton()
{
	OnReleasedDelegate.Broadcast(this);
	OnReleasedDelegateNative.Broadcast(this);
}

void UHorizonButton::OnClickedButton()
{
	OnClickedDelegate.Broadcast(this);
	OnClickedDelegateNative.Broadcast(this);

}


void UHorizonButton::OnHoveredButton()
{
	OnHoveredDelegate.Broadcast(this);
	OnHoveredDelegateNative.Broadcast(this);

}

void UHorizonButton::OnUnhoveredButton()
{
	OnUnhoveredDelegate.Broadcast(this);
	OnUnhoveredDelegateNative.Broadcast(this);
}



#if WITH_EDITOR

const FText UHorizonButton::GetPaletteCategory()
{
	return NSLOCTEXT("HorizonPlugin", "HorizonPlugin", "HorizonPlugin");
}





void UHorizonButton::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) {
	Super::PostEditChangeProperty(PropertyChangedEvent);

}
#endif //#if WITH_EDITOR

