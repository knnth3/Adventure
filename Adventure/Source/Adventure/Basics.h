// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <map>
#include <utility>
#include <memory>
#include <vector>
#include <list>

#include "Adventure.h"
#include "OnlineSubsystemTypes.h"
#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "DrawDebugHelpers.h"
#include "Basics.generated.h"

/**
 * 
 */

const static float CELL_LENGTH_FEET = 5.0f;
const static float CELL_WIDTH_FEET = 5.0f;

typedef std::pair<int, int> CoordinatePair;

template <typename T>
using vector2D = std::vector<std::vector<T>>;

template<typename TEnum>
static FORCEINLINE FString GetEnumValueAsString(const FString& Name, TEnum Value)
{
	const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
	if (!enumPtr)
	{
		return FString("Invalid");
	}
	return enumPtr->GetDisplayNameTextByValue((int64)Value).ToString();
}

static FORCEINLINE FString ParseStringFor(const FString& Text, const FString& Command, const FString& Endline)
{
	FString Result = "";
	if (Text.Split(Command, nullptr, &Result))
	{
		Result.Split(Endline, &Result, nullptr);
	}

	return Result;
}

UENUM()
enum class ESessionState : uint8
{
	/** An online session has not been created yet */
	NoSession,
	/** An online session is in the process of being created */
	Creating,
	/** Session has been created but the session hasn't started (pre match lobby) */
	Pending,
	/** Session has been asked to start (may take time due to communication with backend) */
	Starting,
	/** The current session has started. Sessions with join in progress disabled are no longer joinable */
	InProgress,
	/** The session is still valid, but the session is no longer being played (post match lobby) */
	Ending,
	/** The session is closed and any stats committed */
	Ended,
	/** The session is being destroyed */
	Destroying
};

UENUM()
enum class EJoinSessionResults : uint8
{
	/** The join worked as expected */
	Success,
	/** There are no open slots to join */
	SessionIsFull,
	/** The session couldn't be found on the service */
	SessionDoesNotExist,
	/** There was an error getting the session server's address */
	CouldNotRetrieveAddress,
	/** The user attempting to join is already a member of the session */
	AlreadyInSession,
	/** An error not covered above occurred */
	UnknownError
};

enum class UNITS
{
	CENTIMETERS,
	METERS,
	INCHES,
	FEET,
	YARDS,
};

template <typename T>
struct shared_ptr_compare
{
	bool operator () (std::shared_ptr<T> a, std::shared_ptr<T> b) const
	{
		return !(*a < *b);
	}
};

template <typename T>
struct ptr_compare
{
	bool operator () (T* a, T* b) const
	{
		return !(a->operator<(b));
	}
};

class Conversions
{

public:

	class Centimeters
	{
	public:

		static float ToFeet(const float& Units);

		static float ToInches(const float& Units);

		static float ToYards(const float& Units);

		static float ToMeters(const float& Units);
	};

	class Inches
	{
	public:
		static float ToFeet(const float& Units);

		static float ToCentimeters(const float& Units);

		static float ToYards(const float& Units);

		static float ToMeters(const float& Units);
	};

	class Meters
	{
	public:
		static float ToCentimeters(const float& Units);

		static float ToFeet(const float& Units);

		static float ToInches(const float& Units);

		static float ToYards(const float& Units);

	};

	class Feet
	{
	public:
		static float ToCentimeters(const float& Units);

		static float ToInches(const float& Units);

		static float ToYards(const float& Units);

		static float ToMeters(const float& Units);
	};
};

inline void GetGridDimensions(float& CellLength, float& CellWidth, UNITS units)
{
	switch (units)
	{
	case UNITS::CENTIMETERS:
		CellLength = Conversions::Feet::ToCentimeters(CELL_LENGTH_FEET);
		CellWidth = Conversions::Feet::ToCentimeters(CELL_WIDTH_FEET);
		break;
	case UNITS::METERS:
		CellLength = Conversions::Feet::ToMeters(CELL_LENGTH_FEET);
		CellWidth = Conversions::Feet::ToMeters(CELL_WIDTH_FEET);
		break;
	case UNITS::INCHES:
		CellLength = Conversions::Feet::ToInches(CELL_LENGTH_FEET);
		CellWidth = Conversions::Feet::ToInches(CELL_WIDTH_FEET);
		break;
	case UNITS::FEET:
		CellLength = CELL_LENGTH_FEET;
		CellWidth = CELL_WIDTH_FEET;
		break;
	case UNITS::YARDS:
		CellLength = Conversions::Feet::ToYards(CELL_LENGTH_FEET);
		CellWidth = Conversions::Feet::ToYards(CELL_WIDTH_FEET);
		break;
	default:
		break;
	}
}

inline FString GetStringOf(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "Role: None";
	case ROLE_SimulatedProxy:
		return "Role: Simulated Proxy";
	case ROLE_AutonomousProxy:
		return "Role: Autonomous Proxy";
	case ROLE_Authority:
		return "Role: Authority";
	default:
		return "Role: Error Encountered";
	}
}

USTRUCT(BlueprintType)
struct FProceduralMeshInfo 
{
	GENERATED_BODY()
	
	int Num() const
	{
		return Positions.Num();
	}
	
	void Add(const FVector& Position, const FVector& Normal, const FVector2D& uv, const FLinearColor& Color)
	{
		Positions.Push(Position);
		Normals.Push(Normal);
		UV.Push(uv);
		Colors.Push(Color);
	}
	
	UPROPERTY(BlueprintReadWrite, Category = "Vertex Info")
	TArray<FVector> Positions;

	UPROPERTY(BlueprintReadWrite, Category = "Vertex Info")
	TArray<FVector> Normals;

	UPROPERTY(BlueprintReadWrite, Category = "Vertex Info")
	TArray<FVector2D> UV;

	UPROPERTY(BlueprintReadWrite, Category = "Vertex Info")
	TArray<FLinearColor> Colors;
};

USTRUCT(BlueprintType)
struct ADVENTURE_API FGridCoordinate
{
	GENERATED_BODY()

	FGridCoordinate();
	FGridCoordinate(int32 x, int32 y, int32 z = 0);
	FGridCoordinate(FVector Location3D);
	FGridCoordinate(CoordinatePair Location);

	bool operator==(const FGridCoordinate& b)const;
	bool operator!=(const FGridCoordinate& b)const;
	FGridCoordinate operator+(const FGridCoordinate& b)const;
	FGridCoordinate operator-(const FGridCoordinate& b)const;
	FGridCoordinate operator*(const int32& s)const;

	CoordinatePair toPair()const;
	bool IsZero()const;

	UPROPERTY(BlueprintReadWrite, Category = "GridCoordinate")
	int32 X;

	UPROPERTY(BlueprintReadWrite, Category = "GridCoordinate")
	int32 Y;

	UPROPERTY(BlueprintReadWrite, Category = "GridCoordinate")
	int32 Z;

};

UCLASS()
class UGridFunctions : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GridFunctions")
	static FVector GridToWorldLocation(const FGridCoordinate& Location);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "GridFunctions")
	static FGridCoordinate WorldToGridLocation(const FVector& Location);
};

UCLASS()
class ADVENTURE_API UBasicFunctions : public UObject
{
	GENERATED_BODY()
	
public:

	UFUNCTION(BlueprintCallable, Category = "Basic Functions")
	static bool GetAllSaveGameSlotNames(TArray<FString>& Array, FString Ext);

	UFUNCTION(BlueprintCallable, Category = "Basic Functions")
	static bool SaveFile(class USaveGame* SaveGameObject, const FString& SlotName);

	UFUNCTION(BlueprintCallable, Category = "Basic Functions")
	static USaveGame* LoadFile(const FString& SlotName);

	static ESessionState ToBlueprintType(EOnlineSessionState::Type Type);
	static EJoinSessionResults ToBlueprintType(EOnJoinSessionCompleteResult::Type Type);

	static bool TraceLine(FVector Start, FVector End, UWorld* World, FHitResult& Hit, ETraceTypeQuery TraceChannel = ETraceTypeQuery::TraceTypeQuery1, AActor* Ignore = nullptr, bool bShowTrace = false);
};


class MeshLibrary
{
public:

	static void GenerateGrid(FProceduralMeshInfo& Vertices, TArray<int32>& Triangles,
		int xDivisions, int yDivisions, float Width, float Height, float TopCornerX, float TopCornerY);
};