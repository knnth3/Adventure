// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Attacks/Attack.h"
#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "AttackDatabase.generated.h"


// Lookup result row when using a UDataTable
USTRUCT(Blueprintable)
struct ADVENTURE_API FAttacksRow : public FTableRowBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintReadWrite, EditDefaultsOnly, Category = "InventoryLookUp")
	TSubclassOf<AAttack> AttackClass = nullptr;
};

static UDataTable* BaseAttackTable = nullptr;

UCLASS(Blueprintable)
class ADVENTURE_API UAttackDatabase : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UAttackDatabase();

	static TSubclassOf<AAttack> GetAttack(int AttackID);
};
