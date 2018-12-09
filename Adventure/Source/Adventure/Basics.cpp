// Fill out your copyright notice in the Description page of Project Settings.

#include "Basics.h"
#include "FileManager.h"
#include "Paths.h"
#include "GameFramework/SaveGame.h"
#include "Serialization/MemoryWriter.h"
#include "Serialization/CustomVersion.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "Serialization/MemoryReader.h"
#include "SaveGameSystem.h"
#include "PlatformFeatures.h"
#include "Kismet/KismetSystemLibrary.h"

#define CM_TO_M_FACTOR 100
#define CM_TO_IN_FACTOR 2.54
#define IN_TO_YD_FACTOR 36
#define IN_TO_FT_FACTOR 12

static const int UE4_SAVEGAME_FILE_TYPE_TAG = 0x53415647;		// "sAvG"

struct FSaveGameFileVersionCopy
{
	enum Type
	{
		InitialVersion = 1,
		// serializing custom versions into the savegame data to handle that type of versioning
		AddedCustomVersions = 2,

		// -----<new versions can be added above this line>-------------------------------------------------
		VersionPlusOne,
		LatestVersion = VersionPlusOne - 1
	};
};

struct FSaveGameHeaderCopy
{
	FSaveGameHeaderCopy();
	FSaveGameHeaderCopy(TSubclassOf<USaveGame> ObjectType);

	void Empty();
	bool IsEmpty() const;

	void Read(FMemoryReader& MemoryReader);
	void Write(FMemoryWriter& MemoryWriter);

	int32 FileTypeTag;
	int32 SaveGameFileVersion;
	int32 PackageFileUE4Version;
	FEngineVersion SavedEngineVersion;
	int32 CustomVersionFormat;
	FCustomVersionContainer CustomVersions;
	FString SaveGameClassName;
};

FSaveGameHeaderCopy::FSaveGameHeaderCopy()
	: FileTypeTag(0)
	, SaveGameFileVersion(0)
	, PackageFileUE4Version(0)
	, CustomVersionFormat(static_cast<int32>(ECustomVersionSerializationFormat::Unknown))
{}

FSaveGameHeaderCopy::FSaveGameHeaderCopy(TSubclassOf<USaveGame> ObjectType)
	: FileTypeTag(UE4_SAVEGAME_FILE_TYPE_TAG)
	, SaveGameFileVersion(FSaveGameFileVersionCopy::LatestVersion)
	, PackageFileUE4Version(GPackageFileUE4Version)
	, SavedEngineVersion(FEngineVersion::Current())
	, CustomVersionFormat(static_cast<int32>(ECustomVersionSerializationFormat::Latest))
	, CustomVersions(FCustomVersionContainer::GetRegistered())
	, SaveGameClassName(ObjectType->GetPathName())
{}

void FSaveGameHeaderCopy::Empty()
{
	FileTypeTag = 0;
	SaveGameFileVersion = 0;
	PackageFileUE4Version = 0;
	SavedEngineVersion.Empty();
	CustomVersionFormat = (int32)ECustomVersionSerializationFormat::Unknown;
	CustomVersions.Empty();
	SaveGameClassName.Empty();
}

bool FSaveGameHeaderCopy::IsEmpty() const
{
	return (FileTypeTag == 0);
}

void FSaveGameHeaderCopy::Read(FMemoryReader& MemoryReader)
{
	Empty();

	MemoryReader << FileTypeTag;

	if (FileTypeTag != UE4_SAVEGAME_FILE_TYPE_TAG)
	{
		// this is an old saved game, back up the file pointer to the beginning and assume version 1
		MemoryReader.Seek(0);
		SaveGameFileVersion = FSaveGameFileVersionCopy::InitialVersion;

		// Note for 4.8 and beyond: if you get a crash loading a pre-4.8 version of your savegame file and 
		// you don't want to delete it, try uncommenting these lines and changing them to use the version 
		// information from your previous build. Then load and resave your savegame file.
		//MemoryReader.SetUE4Ver(MyPreviousUE4Version);				// @see GPackageFileUE4Version
		//MemoryReader.SetEngineVer(MyPreviousEngineVersion);		// @see FEngineVersion::Current()
	}
	else
	{
		// Read version for this file format
		MemoryReader << SaveGameFileVersion;

		// Read engine and UE4 version information
		MemoryReader << PackageFileUE4Version;

		MemoryReader << SavedEngineVersion;

		MemoryReader.SetUE4Ver(PackageFileUE4Version);
		MemoryReader.SetEngineVer(SavedEngineVersion);

		if (SaveGameFileVersion >= FSaveGameFileVersionCopy::AddedCustomVersions)
		{
			MemoryReader << CustomVersionFormat;

			CustomVersions.Serialize(MemoryReader, static_cast<ECustomVersionSerializationFormat::Type>(CustomVersionFormat));
			MemoryReader.SetCustomVersions(CustomVersions);
		}
	}

	// Get the class name
	MemoryReader << SaveGameClassName;
}

void FSaveGameHeaderCopy::Write(FMemoryWriter& MemoryWriter)
{
	// write file type tag. identifies this file type and indicates it's using proper versioning
	// since older UE4 versions did not version this data.
	MemoryWriter << FileTypeTag;

	// Write version for this file format
	MemoryWriter << SaveGameFileVersion;

	// Write out engine and UE4 version information
	MemoryWriter << PackageFileUE4Version;
	MemoryWriter << SavedEngineVersion;

	// Write out custom version data
	MemoryWriter << CustomVersionFormat;
	CustomVersions.Serialize(MemoryWriter, static_cast<ECustomVersionSerializationFormat::Type>(CustomVersionFormat));

	// Write the class name so we know what class to load to
	MemoryWriter << SaveGameClassName;
}

FGridCoordinate::FGridCoordinate()
{
	X = 0;
	Y = 0;
	Z = 0;
}

FGridCoordinate::FGridCoordinate(int32 x, int32 y, int32 z)
{
	X = x;
	Y = y;
	Z = z;
}

FGridCoordinate::FGridCoordinate(FVector Location3D)
{
	FGridCoordinate temp = UGridFunctions::WorldToGridLocation(Location3D);
	X = temp.X;
	Y = temp.Y;
	Z = temp.Z;
}

FGridCoordinate::FGridCoordinate(CoordinatePair Location)
{
	X = Location.first;
	Y = Location.second;
	Z = 0.0f;
}

bool FGridCoordinate::operator==(const FGridCoordinate & b)const
{
	return (X == b.X && Y == b.Y);
}

bool FGridCoordinate::operator!=(const FGridCoordinate & b)const
{
	return !operator==(b);
}

FGridCoordinate FGridCoordinate::operator+(const FGridCoordinate & b) const
{
	return FGridCoordinate(X + b.X, Y + b.Y, Z + b.Z);
}

FGridCoordinate FGridCoordinate::operator-(const FGridCoordinate & b) const
{
	return FGridCoordinate(X - b.X, Y - b.Y, Z - b.Z);
}

FGridCoordinate FGridCoordinate::operator*(const int32 & s) const
{
	return FGridCoordinate(X * s, Y * s, Z * s);
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

	float finalX = (Location.X * CELL_LENGTH) + (CELL_LENGTH/ 2.0f);
	float finalY = (Location.Y * CELL_WIDTH) + (CELL_WIDTH / 2.0f);

	return FVector(finalX, finalY, Location.Z);
}

FGridCoordinate UGridFunctions::WorldToGridLocation(const FVector& Location)
{
	//Requres units to be in cm (ue4)
	float CELL_LENGTH, CELL_WIDTH;
	GetGridDimensions(CELL_LENGTH, CELL_WIDTH, UNITS::CENTIMETERS);

	float finalX = ((Location.X) / CELL_LENGTH);
	float finalY = ((Location.Y) / CELL_WIDTH);

	if (Location.X < 0)
	{
		finalX--;
	}
	if (Location.Y < 0)
	{
		finalY--;
	}


	return FGridCoordinate(finalX, finalY, Location.Z);
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
	FString RootFolderFullPath = FPaths::ProjectUserDir() + "maps/";

	if (RootFolderFullPath.Len() < 1) return false;

	FPaths::NormalizeDirectoryName(RootFolderFullPath);

	UE_LOG(LogNotice, Warning, TEXT("Attempting to locate all files in folderpath: %s"), *RootFolderFullPath);

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

bool UBasicFunctions::SaveFile(USaveGame * SaveGameObject, const FString & SlotName)
{
	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	// If we have a system and an object to save and a save name...
	if (SaveSystem && SaveGameObject && (SlotName.Len() > 0))
	{
		TArray<uint8> ObjectBytes;
		FMemoryWriter MemoryWriter(ObjectBytes, true);

		FSaveGameHeaderCopy SaveHeader(SaveGameObject->GetClass());
		SaveHeader.Write(MemoryWriter);

		// Then save the object state, replacing object refs and names with strings
		FObjectAndNameAsStringProxyArchive Ar(MemoryWriter, false);
		SaveGameObject->Serialize(Ar);

		// Stuff that data into the save system with the desired file name
		return FFileHelper::SaveArrayToFile(ObjectBytes, *SlotName);
	}

	return false;
}

USaveGame * UBasicFunctions::LoadFile(const FString & SlotName)
{
	USaveGame* OutSaveGameObject = NULL;

	ISaveGameSystem* SaveSystem = IPlatformFeaturesModule::Get().GetSaveGameSystem();
	// If we have a save system and a valid name..
	if (SaveSystem && (SlotName.Len() > 0))
	{
		// Load raw data from slot
		TArray<uint8> ObjectBytes;
		bool bSuccess = FFileHelper::LoadFileToArray(ObjectBytes, *SlotName);
		if (bSuccess)
		{
			FMemoryReader MemoryReader(ObjectBytes, true);

			FSaveGameHeaderCopy SaveHeader;
			SaveHeader.Read(MemoryReader);

			// Try and find it, and failing that, load it
			UClass* SaveGameClass = FindObject<UClass>(ANY_PACKAGE, *SaveHeader.SaveGameClassName);
			if (SaveGameClass == NULL)
			{
				SaveGameClass = LoadObject<UClass>(NULL, *SaveHeader.SaveGameClassName);
			}

			// If we have a class, try and load it.
			if (SaveGameClass != NULL)
			{
				OutSaveGameObject = NewObject<USaveGame>(GetTransientPackage(), SaveGameClass);

				FObjectAndNameAsStringProxyArchive Ar(MemoryReader, true);
				OutSaveGameObject->Serialize(Ar);
			}
		}
	}

	return OutSaveGameObject;
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

bool UBasicFunctions::TraceLine(FVector Start, FVector End, UWorld* World, FHitResult * RV_Hit, bool bShowTrace, ETraceTypeQuery TraceChannel)
{
	if (World)
	{
		auto showTrace = EDrawDebugTrace::None;
		TArray<AActor*> IgnoreActors;

		if (bShowTrace)
			showTrace = EDrawDebugTrace::ForDuration;

		//  do the line trace
		bool DidTrace = UKismetSystemLibrary::LineTraceSingle(
			World,
			Start,
			End,
			TraceChannel,
			true,
			IgnoreActors,
			showTrace,
			*RV_Hit,
			true,
			FLinearColor::Blue
		);

		return DidTrace;
	}

	return false;
}

void MeshLibrary::GenerateGrid(FProceduralMeshInfo& Vertices, TArray<int32>& Triangles,
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
			float u = (float)x / (xDivisions + 1);
			float v = (float)y / (yDivisions + 1);
			Vertices.Add(
				FVector(xOffset, yOffset, 0),
				FVector(0, 0, 1),
				FVector2D(u, v),
				FColor::White
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
