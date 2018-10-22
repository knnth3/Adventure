// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#include "WorldGrid_Cell.h"
#include "Character/MapPawn.h"

// Sets default values
AWorldGrid_Cell::AWorldGrid_Cell()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
	m_BlockingSpaceCount = 0;

	m_Object = nullptr;
	m_ObjectType = GRID_OBJECT_TYPE::NONE;
}

void AWorldGrid_Cell::Initialize(const FGridCoordinate & newLocation)
{
	Location = newLocation;
}

void AWorldGrid_Cell::SetParent(AWorldGrid * parent)
{
	ParentGrid = parent;
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

bool AWorldGrid_Cell::IsEmpty()const
{
	return m_ObjectType == GRID_OBJECT_TYPE::NONE && m_Pawns.empty();
}

bool AWorldGrid_Cell::IsOcupied() const
{
	bool hasBlockingSpaces = m_BlockingSpaceCount > 0;

	return hasBlockingSpaces || !m_Pawns.empty() || m_Object;
}

bool AWorldGrid_Cell::HasPawn() const
{
	return !m_Pawns.empty();
}

GRID_OBJECT_TYPE AWorldGrid_Cell::GetObjectType() const
{
	if (m_ObjectType == GRID_OBJECT_TYPE::NONE && !m_Pawns.empty())
		return GRID_OBJECT_TYPE::PAWN;

	return m_ObjectType;
}

bool AWorldGrid_Cell::AddObject(GRID_OBJECT_TYPE type, AActor* actor)
{
	if (m_Object || !actor)
	{
		return false;
	}

	switch (type)
	{
	case GRID_OBJECT_TYPE::INTERACTABLE:
		m_Object = actor;
		break;

	case GRID_OBJECT_TYPE::SPAWN:
		m_Object = actor;
		break;

	default:
		return false;
	}

	m_ObjectType = type;
	return true;
}

bool AWorldGrid_Cell::AddPawn(AActor * actor)
{
	if (!actor)
	{
		return false;
	}

	if (Cast<AMapPawn>(actor))
	{
		m_Pawns.push_back(actor);
		return true;
	}

	return false;
}

AActor * AWorldGrid_Cell::RemoveObject()
{
	AActor* removed = m_Object;
	m_Object = nullptr;
	m_ObjectType = GRID_OBJECT_TYPE::NONE;

	return removed;
}

AActor * AWorldGrid_Cell::RemovePawn(int pawnID)
{
	AActor* removed = nullptr;
	if (!m_Pawns.empty())
	{
		//Remove the top pawn
		if (pawnID == -1)
		{
			removed = m_Pawns.back();
			m_Pawns.pop_back();
		}
		else
		{
			// Linear search for correct pawnID
			for (int x = 0; x < m_Pawns.size(); x++)
			{
				AMapPawn* pawn = Cast<AMapPawn>(m_Pawns[x]);
				if (pawn && pawn->GetPawnID() == pawnID)
				{
					removed = m_Pawns[x];
					m_Pawns[x] = m_Pawns.back();
					m_Pawns.pop_back();

				}
			}
		}
	}
	return removed;
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

AActor* AWorldGrid_Cell::GetObject()
{
	return m_Object;
}

AMapPawn * AWorldGrid_Cell::GetPawn(int PawnID)
{
	for (const auto& pawn : m_Pawns)
	{
		AMapPawn* PAWN = Cast<AMapPawn>(pawn);
		if (PAWN && PAWN->GetPawnID() == PawnID)
		{
			return PAWN;
		}
	}

	return nullptr;
}

void AWorldGrid_Cell::GetPawns(TArray<AActor*>& pawns)
{
	for (const auto& pawn : m_Pawns)
	{
		pawns.Push(pawn);
	}
}

void AWorldGrid_Cell::ClearCell(TArray<AActor*>& contents)
{
	for (auto& pawn : m_Pawns)
	{
		contents.Push(pawn);
	}

	contents.Push(m_Object);

	m_Object = nullptr;
	m_Pawns.clear();
	m_ObjectType = GRID_OBJECT_TYPE::NONE;
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

std::list<AWorldGrid_Cell*> AWorldGrid_Cell::GetNeighbors()
{
	std::list<AWorldGrid_Cell*> traversableCells;
	for (int index = 0; index < 8; index++)
	{
		if (Neighbors[index])
		{
			traversableCells.push_back(Neighbors[index]);
		}
	}
	return traversableCells;
}
