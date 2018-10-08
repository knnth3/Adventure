// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Basics.h"
#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GridEntity.generated.h"


UENUM(BlueprintType)
enum class GridEntityType : uint8
{
	NON_BLOCKING,
	BLOCKING,
	SPAWNER,
	CHARACTER,
};

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UGridEntity : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ADVENTURE_API IGridEntity
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Entity Info")
	int GetClassIndex() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Entity Info")
	int GetObjectID() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Entity Info")
	bool IsNonTraversable() const;

	UFUNCTION(BlueprintCallable, BlueprintNativeEvent, Category = "Entity Info")
	bool IsBlockingSpace() const;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Entity Info")
	GridEntityType GetType() const;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Entity Info")
	FGridCoordinate GetLocationGridSpace() const;

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent, Category = "Entity Info")
	FVector GetLocationWorldSpace() const;
};
