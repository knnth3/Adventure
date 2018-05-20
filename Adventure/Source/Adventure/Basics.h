// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Basics.generated.h"

#define TO_CENTIMETERS(M) M * 100.0f
#define TO_METERS(CM) CM / 100.0f

#define CELL_LENGTH (TO_CENTIMETERS(1.0f))
#define CELL_WIDTH (TO_CENTIMETERS(1.0f))

/**
 * 
 */

USTRUCT(BlueprintType)
struct ADVENTURE_API FGridCoordinate
{
	GENERATED_BODY()

	FGridCoordinate();
	FGridCoordinate(int32 x, int32 y);
	FGridCoordinate(FVector Location3D);

	bool operator==(const FGridCoordinate& b);

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
