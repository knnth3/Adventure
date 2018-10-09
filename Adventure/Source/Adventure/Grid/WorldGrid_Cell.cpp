// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "WorldGrid_Cell.h"


// Sets default values
AWorldGrid_Cell::AWorldGrid_Cell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	bTraversable = true;
	bBlockingObject = false;
	m_Visual = { -1, -1 };
	m_BlockingObject = { -1,-1 };
	m_Spawner = { -1,-1 };
	m_BlockingSpaceCount = 0;
}

void AWorldGrid_Cell::Initialize(const FGridCoordinate & newLocation)
{
	Location = newLocation;
}

// Called when the game starts or when spawned
void AWorldGrid_Cell::BeginPlay()
{
	Super::BeginPlay();
	
}

bool AWorldGrid_Cell::operator<(const AWorldGrid_Cell * b)
{
	if (b)
	{
		if (GetFCost() == b->GetFCost())
		{
			return H_Cost <= b->H_Cost;
		}

		return GetFCost() < b->GetFCost();
	}

	return false;
}

int AWorldGrid_Cell::GetFCost() const
{
	return H_Cost + G_Cost;
}

bool AWorldGrid_Cell::IsOcupied() const
{
	bool hasBlockingSpaces = m_BlockingSpaceCount > 0;
	bool hasPawn = ContainsPawn();
	bool hasBlockingObject = ContainsBlockingObject();
	return hasBlockingSpaces || hasPawn || hasBlockingObject || !bTraversable;
}

bool AWorldGrid_Cell::SetTraversable(bool value)
{
	if (value)
	{
		bTraversable = true;
		return true;
	}
	else if (!value && !ContainsPawn())
	{
		bTraversable = false;
		return true;
	}

	return false;
}

bool AWorldGrid_Cell::IsTraversable() const
{
	return bTraversable;
}

bool AWorldGrid_Cell::AddVisual(FCellObject object)
{
	if (!ContainsSpawner() && !ContainsVisuals() && !ContainsBlockingObject())
	{
		m_Visual = object;
		return true;
	}
	return false;
}

int AWorldGrid_Cell::RemoveVisual()
{
	if (ContainsVisuals())
	{
		int index = m_Visual.ObjectID;
		m_Visual = { -1,-1 };
		return index;
	}
	return -1;
}

bool AWorldGrid_Cell::AddBlockingObject(FCellObject object)
{
	if (!ContainsSpawner() && !ContainsVisuals() && !ContainsBlockingObject())
	{
		m_BlockingObject = object;
		bBlockingObject = true;
		return true;
	}
	return false;
}

int AWorldGrid_Cell::RemoveBlockingObject()
{
	if (ContainsBlockingObject())
	{
		int index = m_BlockingObject.ObjectID;
		m_BlockingObject = { -1,-1 };
		bBlockingObject = false;
		return index;
	}
	return -1;
}

bool AWorldGrid_Cell::AddPawn(FCellObject object)
{
	if (!ContainsBlockingObject() && bTraversable)
	{
		m_Pawns.push_back(object);
		return true;
	}
	return false;
}

int AWorldGrid_Cell::RemovePawn(int pawnID)
{
	if (ContainsPawn())
	{
		int index = GetPawnIndex(pawnID, m_Pawns);
		return RemoveElement(index, m_Pawns);
	}
	return -1;
}

bool AWorldGrid_Cell::AddSpawner(FCellObject object)
{
	if (!ContainsSpawner() && !ContainsVisuals() && !ContainsBlockingObject() && bTraversable)
	{
		m_Spawner = object;
		return true;
	}
	return false;
}

int AWorldGrid_Cell::RemoveSpawner()
{
	if (ContainsSpawner())
	{
		int index = m_Spawner.ObjectID;
		m_Spawner = { -1,-1 };
		return index;
	}
	return -1;
}

void AWorldGrid_Cell::AddBlockingSpace()
{
	m_BlockingSpaceCount++;
}

void AWorldGrid_Cell::RemoveBlockingSpace()
{
	if (m_BlockingSpaceCount > 0)
	{
		m_BlockingSpaceCount--;
	}
}

bool AWorldGrid_Cell::ContainsPawn() const
{
	return !m_Pawns.empty();
}

bool AWorldGrid_Cell::ContainsVisuals() const
{
	return m_Visual.ObjectID != -1;
}

bool AWorldGrid_Cell::ContainsBlockingObject() const
{
	return bBlockingObject;
}

bool AWorldGrid_Cell::ContainsSpawner() const
{
	return m_Spawner.ObjectID != -1;
}

AWorldGrid_Cell*& AWorldGrid_Cell::Neigbor(const NEIGHBOR & location)
{
	switch (location)
	{
	case AWorldGrid_Cell::TOP:
		return Neighbors[0];
	case AWorldGrid_Cell::LEFT:
		return Neighbors[1];
	case AWorldGrid_Cell::RIGHT:
		return Neighbors[2];
	case AWorldGrid_Cell::BOTTOM:
		return Neighbors[3];
	case AWorldGrid_Cell::TOPLEFT:
		return Neighbors[4];
	case AWorldGrid_Cell::BOTTOMLEFT:
		return Neighbors[5];
	case AWorldGrid_Cell::TOPRIGHT:
		return Neighbors[6];
	case AWorldGrid_Cell::BOTTOMRIGHT:
		return Neighbors[7];
	default:
		return Neighbors[0];
	}
}

std::list<AWorldGrid_Cell*> AWorldGrid_Cell::GetTraversableNeighbors()
{
	std::list<AWorldGrid_Cell*> traversableCells;
	for (int index = 0; index < 8; index++)
	{
		if (Neighbors[index] && Neighbors[index]->IsTraversable())
		{
			traversableCells.push_back(Neighbors[index]);
		}
	}
	return traversableCells;
}

int AWorldGrid_Cell::RemoveElement(int index, std::vector<FCellObject>& list)
{
	if (index == -1)
	{
		return index;
	}

	if (list.size() == 1)
	{
		FCellObject removing = list[index];
		list.clear();
		return removing.ObjectID;
	}
	else if (list.size() > 1 && list.size() > index && index >= 0)
	{
		FCellObject removing = list[index];
		FCellObject replacement = list.back();
		list[index] = replacement;
		list.pop_back();
		return removing.ObjectID;
	}

	return -1;
}

int AWorldGrid_Cell::GetPawnIndex(int PawnID, std::vector<FCellObject>& list)
{
	for (int index = 0; index < list.size(); index++)
	{
		if (list[index].ObjectID == PawnID)
		{
			return index;
		}
	}
	return -1;
}
