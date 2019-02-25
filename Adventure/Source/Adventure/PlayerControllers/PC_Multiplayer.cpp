// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "PC_Multiplayer.h"
#include "Grid/PathFinder.h"

APC_Multiplayer::APC_Multiplayer()
{
	UniqueID = -1;
	m_ElapsedTime = 0;
}

void APC_Multiplayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	m_ElapsedTime += DeltaTime;

	if (m_ElapsedTime >= 3)
	{
		Client_Ping(FVector::ZeroVector);
	}
}

void APC_Multiplayer::SetMapName(const FString & Name)
{
	m_MapName = Name;
}

void APC_Multiplayer::SetPlayerID(const int ID)
{
	UniqueID = ID;
}

int APC_Multiplayer::GetPlayerID() const
{
	return UniqueID;
}

void APC_Multiplayer::ShowPathfindingDebugLines(bool Value)
{
	FPathFinder::ShowDebugPathLines(Value);
}

void APC_Multiplayer::Client_Ping_Implementation(const FVector & data)
{
	if (!HasAuthority())
	{
		UE_LOG(LogNotice, Warning, TEXT("<PlayerPawn>: Ping"));
	}
}