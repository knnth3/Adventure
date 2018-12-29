// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/DataTable.h"
#include "GameFramework/Actor.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "PawnDatabase.generated.h"

/**
 * 
 */


USTRUCT(Blueprintable)
struct ADVENTURE_API FPawnDefaultStats
{
	GENERATED_BODY()
public:
	// Pawn Name
	UPROPERTY(BlueprintReadWrite)
	FName Name;

	// Max health pool
	UPROPERTY(BlueprintReadWrite)
	int MaxHealth;

	// Class used to identify strength/weaknesses
	UPROPERTY(BlueprintReadWrite)
	FName Class;

	// Class used to identify strength/weaknesses
	UPROPERTY(BlueprintReadWrite)
	FName Race;

	// Strength stat used to calculate certain actions
	UPROPERTY(BlueprintReadWrite)
	uint8 Strength;

	// Dexterity stat used to calculate certain actions
	UPROPERTY(BlueprintReadWrite)
	uint8 Dexterity;

	// Constitutuion stat used to calculate certain actions
	UPROPERTY(BlueprintReadWrite)
	uint8 Constitution;

	// Intelligence stat used to calculate certain actions
	UPROPERTY(BlueprintReadWrite)
	uint8 Intelligence;

	// Wisdom stat used to calculate certain actions
	UPROPERTY(BlueprintReadWrite)
	uint8 Wisdom;

	// Charisma stat used to calculate certain actions
	UPROPERTY(BlueprintReadWrite)
	uint8 Charisma;
};

// Lookup result row when using a UDataTable
USTRUCT(Blueprintable)
struct ADVENTURE_API FPawnHead : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "InventoryLookUp")
	FName Name;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "InventoryLookUp")
	UStaticMesh* Visual = nullptr;
};

// Lookup result row when using a UDataTable
USTRUCT(Blueprintable)
struct ADVENTURE_API FPawnBody : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "InventoryLookUp")
	FName Name;

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "InventoryLookUp")
	USkeletalMesh* Visual = nullptr;
};

UCLASS()
class ADVENTURE_API APawnInfo : public AActor
{
	GENERATED_BODY()
public:
	// Pawn Stats
	UPROPERTY(BlueprintReadWrite)
	FPawnDefaultStats Stats;

	FPawnHead* HeadVisual = nullptr;

	FPawnBody* BodyVisual = nullptr;

};

// Separated data tables for lookup
static UDataTable* PawnHeadsLookUpTable = nullptr;

// Separated data tables for lookup
static UDataTable* PawBodyLookUpTable = nullptr;

UCLASS()
class ADVENTURE_API UPawnDatabase : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UPawnDatabase();

	// Adds a pawn to the database that can be refrenced later to make an instance of it
	UFUNCTION(BlueprintCallable, Category = "Pawn Database")
	static bool AddPawnToDatabase(const FPawnDefaultStats& Defaults, uint8 HeadVisualIndex, uint8 BodyVisualIndex);

	// Gets all pawn names from an instance
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn Database")
	static TArray<FName> GetAllPawnNamesFromDatabase();

	// Get a pawn from he database
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Pawn Database")
	static APawnInfo* GetPawnFromDatabase(FName Name);

	// Removes a pawn from the database
	UFUNCTION(BlueprintCallable, Category = "Pawn Database")
	static bool RemovePawnFromDatabase(FName Name);

	// Clears the database
	UFUNCTION(BlueprintCallable, Category = "Pawn Database")
	static void ClearDatabase();

private:

	static FPawnHead* GetHeadVisual(uint8 VisualIndex);
	static FPawnBody* GetBodyVisual(uint8 VisualIndex);
	static TMap<FName, APawnInfo*> m_Pawns;
	
};
