// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "GI_Adventure.h"
#include "Blueprint/UserWidget.h"
#include "W_PauseMenu.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API UW_PauseMenu : public UUserWidget
{
	GENERATED_BODY()
	
protected:

	virtual bool Initialize()override;
	
public:

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void Activate();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void Deactivate();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void ExitToMainMenu();

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	bool IsActive()const;

	UFUNCTION(BlueprintCallable, Category = "Pause Menu")
	void AddCallbackInterface(UGI_Adventure* Interface);


private:

	bool bIsActive;
	class UGI_Adventure* InstanceInterface;
	
};
