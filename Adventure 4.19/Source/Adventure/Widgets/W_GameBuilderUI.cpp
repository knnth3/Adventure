// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.
#include "W_GameBuilderUI.h"

#include "GI_Adventure.h"

#ifdef DELETE
#undef DELETE
#endif

bool UW_GameBuilderUI::Initialize()
{
	bool result = Super::Initialize();
	bShowNameTags = false;
	bAlreadyInit = false;
	m_LogicalMap = nullptr;

	if (result)
	{
		MaxLogEntries = 30;
	}

	return result;
}


void UW_GameBuilderUI::BeginDestroy()
{
	Super::BeginDestroy();

	if (m_LogicalMap)
	{
		for (int x = 0; x < GridSize.X; x++)
		{
			if (m_LogicalMap[x])
			{
				delete[] m_LogicalMap[x];
				m_LogicalMap[x] = nullptr;
			}
		}

		delete[] m_LogicalMap;
		m_LogicalMap = nullptr;
	}
}

void UW_GameBuilderUI::NativeTick(const FGeometry& MyGeometry, float DeltaTime)
{
	Super::NativeTick(MyGeometry, DeltaTime);
	if (bShowNameTags)
	{
		for (int x = 0; x < GridSize.X; x++)
		{
			for (int y = 0; y < GridSize.Y; y++)
			{
				FGAMEBUILDER_OBJECT cell = m_LogicalMap[x][y];
				if (cell.Type != GAMEBUILDER_OBJECT_TYPE::ANY)
				{
					FString ObjectType = GetEnumValueAsString("GAMEBUILDER_OBJECT_TYPE", cell.Type);
					FVector GridLocation = UGridFunctions::GridToWorldLocation(cell.Location);

					DrawDebugString(GetWorld(), GridLocation, ObjectType, nullptr, FColor::White, DeltaTime);
				}
			}
		}
	}
}

bool UW_GameBuilderUI::SubmitEditEvent(GAMEBUILDER_OBJECT_TYPE Type, const FGAMEBUILDER_EDIT_EVENT& Event)
{
	bool Success = false;
	bool bSpaceOccupied = false;
	bool bSpace2Occupied = false;

	//If there is a selected list
	if (m_LogicalMap)
	{
		switch (Event.Action)
		{
		case GAMEBUILDER_ACTION::DELETE:
			bSpaceOccupied = IsSpaceOccupied(Event.CurrentLocation, Type);
			if (bSpaceOccupied)
			{
				Success = DeleteObject(Event.CurrentLocation);
			}
			else
			{
				UE_LOG(LogNotice, Warning, TEXT("Could not record Delete action. Space seems to be empty."));
			}
			break;
		case GAMEBUILDER_ACTION::CREATE:
		{
			bSpaceOccupied = IsSpaceOccupied(Event.DesiredLocation);
			if (!bSpaceOccupied)
			{
				FGAMEBUILDER_OBJECT newObject;
				newObject.Type = Type;
				newObject.ModelIndex = Event.ModelIndex;
				newObject.Location = Event.DesiredLocation;

				Success = CreateObject(Event.DesiredLocation, newObject);
			}
			else
			{
				UE_LOG(LogNotice, Warning, TEXT("Could not record Create action. Space seems to be occupied."));
			}
		}
		break;
		case GAMEBUILDER_ACTION::MOVE:
			bSpaceOccupied = IsSpaceOccupied(Event.CurrentLocation, Type);
			bSpace2Occupied = IsSpaceOccupied(Event.DesiredLocation);
			if (bSpaceOccupied && !bSpace2Occupied)
			{
				FGAMEBUILDER_OBJECT newObject;
				newObject.Type = Type;
				newObject.ModelIndex = Event.ModelIndex;
				newObject.Location = Event.DesiredLocation;

				if (DeleteObject(Event.CurrentLocation))
				{
					Success = CreateObject(Event.DesiredLocation, newObject);
				}
			}
			else
			{
				UE_LOG(LogNotice, Warning, TEXT("Could not record Move action. Desired Space seems to be occupied."));
			}
			break;
		default:
			break;
		}

		if (Success)
		{
			m_editLog.emplace_back(Event);
			HandleLogOverflow();
		}
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("Could not record any actions. UI Grid has not be created."));
	}

	return Success;
}

FGAMEBUILDER_EDIT_EVENT UW_GameBuilderUI::GetPreviousEvent()
{
	FGAMEBUILDER_EDIT_EVENT prev;
	if (!m_editLog.empty())
	{
		prev = m_editLog.back();
		m_editLog.pop_back();
	}

	return prev;
}

void UW_GameBuilderUI::SetMapName(const FName & MapName)
{
	m_mapName = MapName;
}

FName UW_GameBuilderUI::GetMapName()
{
	return m_mapName;
}

bool UW_GameBuilderUI::CreateGrid(const FGridCoordinate & Size)
{
	if (Size.IsZero())
	{
		UE_LOG(LogNotice, Warning, TEXT("Attempted to create UI Grid with invalid sizes: %i x %i"), Size.X, Size.Y);
		return false;
	}

	if (!bAlreadyInit)
	{
		bAlreadyInit = true;
		GridSize = Size;
		//Initialize a 2D array to hold object Info
		m_LogicalMap = new FGAMEBUILDER_OBJECT*[Size.X]();
		for (int x = 0; x < Size.X; x++)
		{
			m_LogicalMap[x] = new FGAMEBUILDER_OBJECT[Size.Y]();
		}
		return true;
	}
	else
	{
		UE_LOG(LogNotice, Warning, TEXT("Could not create UI Grid. It already exists."));
	}

	return false;
}

void UW_GameBuilderUI::ClearGrid()
{
	if (m_LogicalMap)
	{
		for (int x = 0; x < GridSize.X; x++)
		{
			if (m_LogicalMap[x])
			{
				for (int y = 0; y < GridSize.Y; y++)
				{
					if (m_LogicalMap[x][y].Type != GAMEBUILDER_OBJECT_TYPE::ANY)
					{
						FGAMEBUILDER_EDIT_EVENT clearEvent;
						clearEvent.Action = GAMEBUILDER_ACTION::DELETE;
						clearEvent.CurrentLocation = m_LogicalMap[x][y].Location;

						SubmitEditEvent(m_LogicalMap[x][y].Type, clearEvent);
					}
				}
			}
		}
	}
}

void UW_GameBuilderUI::GetObjecteByCoordinate(const FGridCoordinate & coordinate, FGAMEBUILDER_OBJECT& Type)const
{
	Type = FGAMEBUILDER_OBJECT();

	FGAMEBUILDER_OBJECT* Obj = At(coordinate);
	if (Obj)
	{
		Type = *Obj;
	}
}

void UW_GameBuilderUI::GetAllObjects(TArray<FGAMEBUILDER_OBJECT>& Objects)const
{
	if (m_LogicalMap)
	{
		UE_LOG(LogNotice, Warning, TEXT("Pulling all Objects within the UI Grid. Size = %s x %s"), *FString::FromInt(GridSize.X), *FString::FromInt(GridSize.Y));

		for (int x = 0; x < GridSize.X; x++)
		{
			if (m_LogicalMap[x])
			{
				for (int y = 0; y < GridSize.Y; y++)
				{
					if (m_LogicalMap[x][y].Type != GAMEBUILDER_OBJECT_TYPE::ANY)
					{
						Objects.Push(m_LogicalMap[x][y]);
					}
				}
			}
		}
	}
}

bool UW_GameBuilderUI::DeleteObject(const FGridCoordinate& Location)
{
	FGAMEBUILDER_OBJECT* Obj = At(Location);
	if (Obj)
	{
		Obj->Type = GAMEBUILDER_OBJECT_TYPE::ANY;
		return true;
	}

	return false;
}

bool UW_GameBuilderUI::CreateObject(const FGridCoordinate & Location, const FGAMEBUILDER_OBJECT& Object)
{
	FGAMEBUILDER_OBJECT* Obj = At(Location);
	if (Obj)
	{
		*Obj = Object;

		return true;
	}

	return false;
}

void UW_GameBuilderUI::HandleLogOverflow()
{
	if (m_editLog.size() > MaxLogEntries)
	{
		m_editLog.pop_front();
	}
}

bool UW_GameBuilderUI::IsSpaceOccupied(const FGridCoordinate & coordinate, GAMEBUILDER_OBJECT_TYPE Type)
{
	FGAMEBUILDER_OBJECT* Obj = At(coordinate);
	return (Obj && Type != GAMEBUILDER_OBJECT_TYPE::ANY && Obj->Type == Type);
}

FGAMEBUILDER_OBJECT * UW_GameBuilderUI::At(const FGridCoordinate & Location)const
{
	//Check if coordinates are in bounds
	bool XInBounds = (Location.X >= 0 && Location.X < GridSize.X);
	bool YInBounds = (Location.Y >= 0 && Location.Y < GridSize.Y);

	if (XInBounds && YInBounds)
	{
		return &m_LogicalMap[Location.X][Location.Y];
	}

	return nullptr;
}
