// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "W_Lobby.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API UW_Lobby : public UUserWidget
{
	GENERATED_BODY()
	
public:

	virtual bool Initialize()override;

	bool Activate();
	void Deactivate();
	void AddCharacter(FString Username);
	void AddServerCallback(class AGM_Lobby* Gamemode);


	//Blueprint Functions
	UFUNCTION(BlueprintImplementableEvent, Category = "Lobby Menu")
	void OnCharacterConnected(const FString& Username);

	UFUNCTION(BlueprintCallable, Category = "Lobby UI")
	void StartSession();

private:

	class AGM_Lobby* m_Gamemode;
};
