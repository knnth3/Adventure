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

#include "PawnDatabase.h"
#include "Adventure.h"

TMap<FName, APawnInfo*> UPawnDatabase::m_Pawns = TMap<FName, APawnInfo*>();

UPawnDatabase::UPawnDatabase()
{
	static::ConstructorHelpers::FObjectFinder<UDataTable> PawnHeadTable(TEXT("/Game/Databases/DB_PawnHeads"));
	if (PawnHeadTable.Object)
	{
		PawnHeadsLookUpTable = PawnHeadTable.Object;
	}

	static::ConstructorHelpers::FObjectFinder<UDataTable> PawnBodyTable(TEXT("/Game/Databases/DB_PawnBodies"));
	if (PawnBodyTable.Object)
	{
		PawBodyLookUpTable = PawnBodyTable.Object;
	}
}

bool UPawnDatabase::AddPawnToDatabase(const FPawnDefaultStats& Defaults, uint8 HeadVisualIndex, uint8 BodyVisualIndex)
{
	if (GEngine && GEngine->GameViewport)
	{
		UWorld* wworld = GEngine->GameViewport->GetWorld();
		if (wworld && !m_Pawns.Find(Defaults.Name))
		{
			APawnInfo* newPawn = wworld->SpawnActor<APawnInfo>();
			newPawn->Stats = Defaults;
			newPawn->HeadVisual = GetHeadVisual(HeadVisualIndex);
			newPawn->BodyVisual = GetBodyVisual(HeadVisualIndex);
			m_Pawns.Emplace(Defaults.Name, newPawn);

			return true;
		}
	}

	return false;
}

TArray<FName> UPawnDatabase::GetAllPawnNamesFromDatabase()
{
	TArray<FName> Names;
	for (const auto& name : m_Pawns)
	{
		Names.Push(name.Key);
	}

	return Names;
}

APawnInfo * UPawnDatabase::GetPawnFromDatabase(FName Name)
{
	if (m_Pawns.Find(Name))
	{
		return m_Pawns[Name];
	}

	return nullptr;
}

bool UPawnDatabase::RemovePawnFromDatabase(FName Name)
{
	if (m_Pawns.Find(Name))
	{
		m_Pawns[Name]->Destroy();
		m_Pawns.Remove(Name);
	}
	return false;
}

void UPawnDatabase::ClearDatabase()
{
	for (const auto& pawn : m_Pawns)
	{
		pawn.Value->Destroy();
	}

	m_Pawns.Empty();
}

FPawnHead * UPawnDatabase::GetHeadVisual(uint8 VisualIndex)
{
	FPawnHead* FoundRow = nullptr;
	FName RowName = *FString::FromInt((int)VisualIndex);
	FoundRow = PawnHeadsLookUpTable->FindRow<FPawnHead>(RowName, FString("GENERAL"));

	return FoundRow;
}

FPawnBody * UPawnDatabase::GetBodyVisual(uint8 VisualIndex)
{
	FPawnBody* FoundRow = nullptr;
	FName RowName = *FString::FromInt((int)VisualIndex);
	FoundRow = PawBodyLookUpTable->FindRow<FPawnBody>(RowName, FString("GENERAL"));

	return FoundRow;
}
