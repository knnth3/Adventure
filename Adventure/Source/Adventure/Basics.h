// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <utility>
#include <memory>
#include <vector>

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
struct ADVENTURE_API FGridCoordinate
{
	GENERATED_BODY()

	FGridCoordinate();
	FGridCoordinate(int32 x, int32 y);
	FGridCoordinate(FVector Location3D);

	bool operator==(const FGridCoordinate& b);
	bool operator!=(const FGridCoordinate& b);
	CoordinatePair toPair()const;

	UPROPERTY(BlueprintReadWrite, Category = "GridCoordinate")
	int32 X;

	UPROPERTY(BlueprintReadWrite, Category = "GridCoordinate")
	int32 Y;

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
	
	static FORCEINLINE bool LineTraceByChannel(
			UWorld* World,
			AActor* ActorToIgnore,
			const FVector& Start,
			const FVector& End,
			FHitResult& HitOut,
			ECollisionChannel CollisionChannel = ECC_Pawn,
			bool ReturnPhysMat = false
		)
	{
		if (!World)
		{
			return false;
		}

		FCollisionQueryParams TraceParams(FName(TEXT("VictoreCore Trace")), true, ActorToIgnore);
		TraceParams.bTraceComplex = true;
		//TraceParams.bTraceAsyncScene = true;
		TraceParams.bReturnPhysicalMaterial = ReturnPhysMat;

		//Ignore Actors
		TraceParams.AddIgnoredActor(ActorToIgnore);

		//Re-initialize hit info
		HitOut = FHitResult(ForceInit);

		//Trace!
		World->LineTraceSingleByChannel(
			HitOut,		//result
			Start,	//start
			End, //end
			CollisionChannel, //collision channel
			TraceParams
		);

		//Hit any Actor?
		return (HitOut.GetActor() != NULL);
	}
	
	
};
