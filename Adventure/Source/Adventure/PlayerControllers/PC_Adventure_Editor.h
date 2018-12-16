// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Basics.h"
#include "CoreMinimal.h"
#include "Widgets/W_GameBuilderUI.h"
#include "GameFramework/PlayerController.h"
#include "PC_Adventure_Editor.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API APC_Adventure_Editor : public APlayerController
{
	GENERATED_BODY()
	
protected:
	//Sends request to WorldGrid
	UFUNCTION(BlueprintCallable)
	bool RequestSpawnBlockingObject(uint8 Type, const FTransform& Transform);

	UFUNCTION(BlueprintCallable)
	bool RequestAddSpawnLocation(int Type, const FGridCoordinate& Location);

	UFUNCTION(BlueprintCallable)
	bool RequestDeleteObject(enum GRID_OBJECT_TYPE Type, const FGridCoordinate& Location, int ID = -1);
	
};
