// Copyright Epic Games, Inc. All Rights Reserved.

#include "FocusDialog.h"

DECLARE_LOG_CATEGORY_CLASS(LogFocusDialog, Log, All);

void UFocusDialog::InitializeFocusDialog(const FOnCloseDialogDelegate& InCloseDialogDelegate, int32 InZOrder, bool bStartClosed)
{
	OnCloseDialogDelegate = InCloseDialogDelegate;
	ZOrder = InZOrder;

	if (bStartClosed && !bRemoveOnClose)
	{
		AddToPlayerScreen(ZOrder);
		SetVisibility(ESlateVisibility::Collapsed);
	}
	else
	{
		Show();
	}
}

void UFocusDialog::RemoveFromParent()
{
	if (!IsClosed())
	{
		// don't call Close because we don't want to call RemoveFromParent in a loop
		Close_Internal();
	}

	Super::RemoveFromParent();
}

void UFocusDialog::Close()
{
	if (IsClosed())
	{
		return;
	}

	Close_Internal();

	if (bRemoveOnClose)
	{
		RemoveFromParent();
	}
	else
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UFocusDialog::Close_Internal()
{
	bClosed = true;
	OnCloseDialogDelegate.Execute(this);
}

void UFocusDialog::Show()
{
	if (!IsClosed())
	{
		return;
	}

	bClosed = false;

	if (bRemoveOnClose)
	{
		AddToPlayerScreen(ZOrder);
	}
	else
	{
		SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	SetFocus();
}

void UFocusDialog::Cancel()
{
	Close();
}

void UFocusDialog::Confirm()
{
	Close();
}
