// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "PC_Multiplayer.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API APC_Multiplayer : public APlayerController
{
	GENERATED_BODY()
public:
	APC_Multiplayer();
	void SetPlayerID(const int ID);
	int GetPlayerID()const;

private:
	
	UPROPERTY()
	int UniqueID;
	
	
};
