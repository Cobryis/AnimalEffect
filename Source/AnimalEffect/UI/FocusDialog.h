// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Blueprint/UserWidget.h"

#include "FocusDialog.generated.h"

DECLARE_DELEGATE_OneParam(FOnCloseDialogDelegate, UFocusDialog*);

UCLASS()
class ANIMALEFFECT_API UFocusDialog : public UUserWidget
{
	GENERATED_BODY()

public:

	virtual void InitializeFocusDialog(const FOnCloseDialogDelegate& InCloseDialogDelegate, int32 InZOrder, bool bStartClosed);

	void RemoveFromParent() override;

	void Show();
	void Close();
	FORCEINLINE bool IsClosed() const { return bClosed; }

protected:

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	virtual void Cancel();

	UFUNCTION(BlueprintCallable, Category = "Dialog")
	virtual void Confirm();

private:

	void Close_Internal();

private:

	// if true, remove dialog on close instead of collapse. #note: untested
	UPROPERTY(EditAnywhere, Category = "Dialog")
	bool bRemoveOnClose = false;

	int32 ZOrder = 0;

	UPROPERTY(Transient)
	bool bClosed = true;
	
	FOnCloseDialogDelegate OnCloseDialogDelegate;

};
