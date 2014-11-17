// Copyright 1998-2014 Epic Games, Inc. All Rights Reserved.

#include "SlatePrivatePCH.h"
#include "AnalogCursor.h"

#include "HittestGrid.h"

const float FAnalogCursor::Acceleration          = 150.0f;
const float FAnalogCursor::Decceleration         = 250.0f;
const float FAnalogCursor::MaxSpeed			     = 250.0f;
const float FAnalogCursor::OverWidgetMultiplier  = 0.6f;

FAnalogCursor::FAnalogCursor()
: AnalogValues(FVector2D::ZeroVector)
, CurrentSpeed(FVector2D::ZeroVector)
{
}

void FAnalogCursor::Tick(const float DeltaTime, FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor)
{
	FVector2D OldPos = Cursor->GetPosition();

	float SpeedMult = 1.0f; // Used to do a speed multiplication before adding the delta to the position to make widgets sticky
	FVector2D AdjAnalogVals = AnalogValues; // A copy of the analog values so I can modify them based being over a widget, not currently doing this

	FWidgetPath WidgetPath = SlateApp.LocateWindowUnderMouse(OldPos, SlateApp.GetInteractiveTopLevelWindows());
	if (WidgetPath.IsValid())
	{
		FArrangedWidget& ArrangedWidget = WidgetPath.Widgets.Last();
		TSharedRef<SWidget> Widget = ArrangedWidget.Widget;
		if (Widget->SupportsKeyboardFocus() && Widget->GetType() != FName("SViewport"))
		{
			SpeedMult = 0.5f;
			//FVector2D Adjustment = WidgetsAndCursors.Last().Geometry.Position - OldPos; // example of calculating distance from cursor to widget center
		}
	}

	// Generate Min and Max for X to clamp the speed, this gives us instant direction change when crossing the axis
	float CurrentMinSpeedX = 0.0f;
	float CurrentMaxSpeedX = 0.0f;
	if (AdjAnalogVals.X > 0.0f)
	{ 
		CurrentMaxSpeedX = AdjAnalogVals.X * MaxSpeed;
	}
	else
	{
		CurrentMinSpeedX = AdjAnalogVals.X * MaxSpeed;
	}

	// Generate Min and Max for Y to clamp the speed, this gives us instant direction change when crossing the axis
	float CurrentMinSpeedY = 0.0f;
	float CurrentMaxSpeedY = 0.0f;
	if (AdjAnalogVals.Y > 0.0f)
	{
		CurrentMaxSpeedY = AdjAnalogVals.Y * MaxSpeed;
	}
	else
	{
		CurrentMinSpeedY = AdjAnalogVals.Y * MaxSpeed;
	}

	// Cubic acceleration curve
	FVector2D ExpAcceleration = AdjAnalogVals * AdjAnalogVals * AdjAnalogVals * Acceleration;
	// Preserve direction (if we use a squared equation above)
	//ExpAcceleration.X *= FMath::Sign(AnalogValues.X);
	//ExpAcceleration.Y *= FMath::Sign(AnalogValues.Y);

	CurrentSpeed += ExpAcceleration * DeltaTime;

	CurrentSpeed.X = FMath::Clamp(CurrentSpeed.X, CurrentMinSpeedX, CurrentMaxSpeedX);
	CurrentSpeed.Y = FMath::Clamp(CurrentSpeed.Y, CurrentMinSpeedY, CurrentMaxSpeedY);

	const FVector2D NewPosition = OldPos + (CurrentSpeed * SpeedMult);

	//update the cursor position
	UpdateCursorPosition(SlateApp, Cursor, NewPosition);
}

bool FAnalogCursor::HandleKeyDownEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	FKey Key = InKeyEvent.GetKey();

	// Consume the sticks input so it doesn't effect other things
	if (Key == EKeys::Gamepad_LeftStick_Right || 
		Key == EKeys::Gamepad_LeftStick_Left ||
		Key == EKeys::Gamepad_LeftStick_Up ||
		Key == EKeys::Gamepad_LeftStick_Down)
	{
		return true;
	}

	// Bottom face button is a click
	if (Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		FPointerEvent MouseEvent(
			0,
			SlateApp.GetCursorPos(),
			SlateApp.GetLastCursorPos(),
			SlateApp.PressedMouseButtons,
			EKeys::LeftMouseButton,
			0,
			SlateApp.GetPlatformApplication()->GetModifierKeys()
			);

		TSharedPtr<FGenericWindow> GenWindow;
		SlateApp.ProcessMouseButtonDownEvent(GenWindow, MouseEvent);
	}

	return false;
}

bool FAnalogCursor::HandleKeyUpEvent(FSlateApplication& SlateApp, const FKeyEvent& InKeyEvent)
{
	FKey Key = InKeyEvent.GetKey();

	// Consume the sticks input so it doesn't effect other things
	if (Key == EKeys::Gamepad_LeftStick_Right ||
		Key == EKeys::Gamepad_LeftStick_Left ||
		Key == EKeys::Gamepad_LeftStick_Up ||
		Key == EKeys::Gamepad_LeftStick_Down)
	{
		return true;
	}

	// Bottom face button is a click
	if (Key == EKeys::Gamepad_FaceButton_Bottom)
	{
		FPointerEvent MouseEvent(
			0,
			SlateApp.GetCursorPos(),
			SlateApp.GetLastCursorPos(),
			SlateApp.PressedMouseButtons,
			EKeys::LeftMouseButton,
			0,
			SlateApp.GetPlatformApplication()->GetModifierKeys()
			);

		SlateApp.ProcessMouseButtonUpEvent(MouseEvent);
	}
	
	return false;
}

bool FAnalogCursor::HandleAnalogInputEvent(FSlateApplication& SlateApp, const FAnalogInputEvent& InAnalogInputEvent)
{
	FKey Key = InAnalogInputEvent.GetKey();
	float AnalogValue = InAnalogInputEvent.GetAnalogValue();

	//@Todo Slate: Investigate this more, doesn't seem right that analog doesn't zero
	// Analog doesn't zero out
	if (FMath::Abs(AnalogValue) < 0.1f)
	{
		AnalogValue = 0.0f;
	}

	if (Key == EKeys::Gamepad_LeftX)
	{
		AnalogValues.X = AnalogValue;
	}
	else if (Key == EKeys::Gamepad_LeftY)
	{
		AnalogValues.Y = -AnalogValue;
	}
	else
	{
		return false;
	}
	return true;
}

void FAnalogCursor::UpdateCursorPosition(FSlateApplication& SlateApp, TSharedRef<ICursor> Cursor, const FVector2D& NewPosition)
{
	//grab the old position
	const FVector2D OldPosition = Cursor->GetPosition();

	//make sure we are actually moving
	if (OldPosition != NewPosition)
	{
		//put the cursor in the correct spot
		Cursor->SetPosition(NewPosition.X, NewPosition.Y);

		//create a new mouse event
		FPointerEvent MouseEvent(
			0,
			NewPosition,
			OldPosition,
			SlateApp.PressedMouseButtons,
			EKeys::Invalid,
			0,
			SlateApp.GetPlatformApplication()->GetModifierKeys()
			);

		
		//process the event
		SlateApp.ProcessMouseMoveEvent(MouseEvent);
	}
}
