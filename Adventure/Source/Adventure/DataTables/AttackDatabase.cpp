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

#include "AttackDatabase.h"
#include "Adventure.h"

UAttackDatabase::UAttackDatabase()
{
	static::ConstructorHelpers::FObjectFinder<UDataTable> AttackTable(TEXT("/Game/Databases/DB_BaseAttacks"));
	if (AttackTable.Object)
	{
		BaseAttackTable = AttackTable.Object;
	}
}

TSubclassOf<AAttack> UAttackDatabase::GetAttack(int AttackID)
{
	FName RowName = *FString::FromInt((int)AttackID);
	FAttacksRow* FoundRow = BaseAttackTable->FindRow<FAttacksRow>(RowName, FString("GENERAL"));
	if (FoundRow)
	{
		return FoundRow->AttackClass;
	}

	return TSubclassOf<AAttack>(nullptr);
}
