// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

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
