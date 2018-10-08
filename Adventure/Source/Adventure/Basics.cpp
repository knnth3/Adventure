// Fill out your copyright notice in the Description page of Project Settings.

#include "Basics.h"
#include "FileManager.h"
#include "Paths.h"
#include "RuntimeMeshComponent.h"

#define CM_TO_M_FACTOR 100
#define CM_TO_IN_FACTOR 2.54
#define IN_TO_YD_FACTOR 36
#define IN_TO_FT_FACTOR 12

FGridCoordinate::FGridCoordinate()
{
	X = 0;
	Y = 0;
}

FGridCoordinate::FGridCoordinate(int32 x, int32 y)
{
	X = x;
	Y = y;
}

FGridCoordinate::FGridCoordinate(FVector Location3D)
{
	FGridCoordinate temp = UGridFunctions::WorldToGridLocation(Location3D);
	X = temp.X;
	Y = temp.Y;
}

FGridCoordinate::FGridCoordinate(CoordinatePair Location)
{
	X = Location.first;
	Y = Location.second;
}

bool FGridCoordinate::operator==(const FGridCoordinate & b)const
{
	return (X == b.X && Y == b.Y);
}

bool FGridCoordinate::operator!=(const FGridCoordinate & b)const
{
	return !operator==(b);
}

CoordinatePair FGridCoordinate::toPair()const
{
	return CoordinatePair(X, Y);
}

bool FGridCoordinate::IsZero() const
{
	return !(X && Y);
}

FVector UGridFunctions::GridToWorldLocation(const FGridCoordinate& Location)
{
	//Requres units to be in cm (ue4)
	float CELL_LENGTH, CELL_WIDTH;
	GetGridDimensions(CELL_LENGTH, CELL_WIDTH, UNITS::CENTIMETERS);

	return FVector(
		-(Location.X * CELL_LENGTH) - (CELL_LENGTH * 0.5f), 
		(Location.Y * CELL_WIDTH) + (CELL_WIDTH * 0.5f),
		0.0f
	);
}

FGridCoordinate UGridFunctions::WorldToGridLocation(const FVector& Location)
{
	//Requres units to be in cm (ue4)
	float CELL_LENGTH, CELL_WIDTH;
	GetGridDimensions(CELL_LENGTH, CELL_WIDTH, UNITS::CENTIMETERS);

	return FGridCoordinate(
		-FMath::TruncToInt(Location.X / CELL_LENGTH),
		FMath::TruncToInt(Location.Y / CELL_WIDTH)
	);
}

//Centimeters//////////////////////////////////////////////////////////////

float Conversions::Centimeters::ToInches(const float & Units)
{
	return Units / CM_TO_IN_FACTOR;
}

float Conversions::Centimeters::ToYards(const float & Units)
{
	float in = Conversions::Centimeters::ToInches(Units);

	return Conversions::Inches::ToYards(in);
}

float Conversions::Centimeters::ToMeters(const float & Units)
{
	return Units / CM_TO_M_FACTOR;
}

float Conversions::Centimeters::ToFeet(const float & Units)
{
	float in = Conversions::Centimeters::ToInches(Units);

	return Conversions::Inches::ToFeet(in);
}

//Inches////////////////////////////////////////////////////////////////////

float Conversions::Inches::ToFeet(const float & Units)
{
	return Units / IN_TO_FT_FACTOR;
}

float Conversions::Inches::ToCentimeters(const float & Units)
{
	return Units * CM_TO_IN_FACTOR;
}

float Conversions::Inches::ToYards(const float & Units)
{
	return Units / IN_TO_YD_FACTOR;
}

float Conversions::Inches::ToMeters(const float & Units)
{
	float cm = Conversions::Inches::ToCentimeters(Units);

	return Conversions::Centimeters::ToMeters(cm);
}

//Meters/////////////////////////////////////////////////////////////////////

float Conversions::Meters::ToCentimeters(const float & Units)
{
	return Units * CM_TO_M_FACTOR;
}

float Conversions::Meters::ToFeet(const float & Units)
{
	float cm = Conversions::Meters::ToCentimeters(Units);

	return Conversions::Centimeters::ToFeet(cm);
}

float Conversions::Meters::ToInches(const float & Units)
{
	float cm = Conversions::Meters::ToCentimeters(Units);

	return Conversions::Centimeters::ToInches(cm);
}

float Conversions::Meters::ToYards(const float & Units)
{
	float cm = Conversions::Meters::ToCentimeters(Units);

	return Conversions::Centimeters::ToYards(cm);
}

//Feet//////////////////////////////////////////////////////////////////

float Conversions::Feet::ToCentimeters(const float & Units)
{
	float in = Conversions::Feet::ToInches(Units);

	return Conversions::Inches::ToCentimeters(in);
}

float Conversions::Feet::ToInches(const float & Units)
{
	return Units * IN_TO_FT_FACTOR;
}

float Conversions::Feet::ToYards(const float & Units)
{
	float in = Conversions::Feet::ToInches(Units);

	return Conversions::Inches::ToYards(in);
}

float Conversions::Feet::ToMeters(const float & Units)
{
	float in = Conversions::Feet::ToInches(Units);

	return Conversions::Inches::ToMeters(in);
}

bool UBasicFunctions::GetAllSaveGameSlotNames(TArray<FString>& Array, FString Ext)
{
	FString RootFolderFullPath = FPaths::ProjectSavedDir() + "/SaveGames/";

	if (RootFolderFullPath.Len() < 1) return false;

	FPaths::NormalizeDirectoryName(RootFolderFullPath);

	UE_LOG(LogNotice, Warning, TEXT("Attempting to locate all filed in folderpath: %s"), *RootFolderFullPath);

	IFileManager& FileManager = IFileManager::Get();

	if (Ext == "")
	{
		Ext = "*.*";
	}
	else
	{
		Ext = (Ext.Left(1) == ".") ? "*" + Ext : "*." + Ext;
	}

	FString FinalPath = RootFolderFullPath + "/" + Ext;
	FileManager.FindFiles(Array, *FinalPath, true, false);
	return true;
}

ESessionState UBasicFunctions::ToBlueprintType(EOnlineSessionState::Type Type)
{
	ESessionState State = ESessionState::NoSession;
	switch (Type)
	{
	case EOnlineSessionState::NoSession:
		State = ESessionState::NoSession;
		break;
	case EOnlineSessionState::Creating:
		State = ESessionState::Creating;
		break;
	case EOnlineSessionState::Pending:
		State = ESessionState::Pending;
		break;
	case EOnlineSessionState::Starting:
		State = ESessionState::Starting;
		break;
	case EOnlineSessionState::InProgress:
		State = ESessionState::InProgress;
		break;
	case EOnlineSessionState::Ending:
		State = ESessionState::Ending;
		break;
	case EOnlineSessionState::Ended:
		State = ESessionState::Ended;
		break;
	case EOnlineSessionState::Destroying:
		State = ESessionState::Destroying;
		break;
	default:
		break;
	}

	return State;
}

EJoinSessionResults UBasicFunctions::ToBlueprintType(EOnJoinSessionCompleteResult::Type Type)
{
	EJoinSessionResults State = EJoinSessionResults::UnknownError;

	switch (Type)
	{
	case EOnJoinSessionCompleteResult::Success:
		State = EJoinSessionResults::Success;
		break;
	case EOnJoinSessionCompleteResult::SessionIsFull:
		State = EJoinSessionResults::SessionIsFull;
		break;
	case EOnJoinSessionCompleteResult::SessionDoesNotExist:
		State = EJoinSessionResults::SessionDoesNotExist;
		break;
	case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
		State = EJoinSessionResults::CouldNotRetrieveAddress;
		break;
	case EOnJoinSessionCompleteResult::AlreadyInSession:
		State = EJoinSessionResults::AlreadyInSession;
		break;
	case EOnJoinSessionCompleteResult::UnknownError:
		State = EJoinSessionResults::UnknownError;
		break;
	default:
		break;
	}

	return State;
}

void MeshLibrary::GenerateGrid(TArray<struct FRuntimeMeshVertexSimple>& Vertices, TArray<int32>& Triangles, 
	int xDivisions, int yDivisions, float Width, float Height, float TopCornerX, float TopCornerY)
{
	if (Width * Height == 0)
		return;

	bool bFlipTangent = (Width * Height) > 0 ? true : false;
	float deltaWidth = ((float)Width) / (xDivisions + 1);
	float deltaHeight = ((float)Height) / (yDivisions + 1);

	float xOffset = TopCornerX;
	float yOffset = TopCornerY;

	int indexOffset = Vertices.Num();
	int maxYOffset = yDivisions + 1;

	for (int x = 0; x <= (xDivisions + 1); x++)
	{
		for (int y = 0; y <= (yDivisions + 1); y++)
		{
			float u = xOffset / Width;
			float v = yOffset / Width;
			Vertices.Add(
				FRuntimeMeshVertexSimple(
					FVector(xOffset, yOffset, 0), 
					FVector(0, 0, 1), 
					FRuntimeMeshTangent(0, -1, 0, bFlipTangent), 
					FColor::White, 
					FVector2D(u, v)
				)
			);

			yOffset += deltaHeight;

			if (x != 0 && y != 0)
			{
				Triangles.Add(indexOffset + 0);
				Triangles.Add(indexOffset + 1);
				Triangles.Add(indexOffset + maxYOffset + 2);

				Triangles.Add(indexOffset + 0);
				Triangles.Add(indexOffset + maxYOffset + 2);
				Triangles.Add(indexOffset + maxYOffset + 1);
				indexOffset++;
			}
		}

		yOffset = TopCornerY;
		xOffset += deltaWidth;

		if (x != 0)
		{
			indexOffset++;
		}
	}
}
