// Copyright 2019 Eric Marquez
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
