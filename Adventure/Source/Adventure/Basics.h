// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "Engine/EngineTypes.h"
#include "Engine/World.h"
#include "CollisionQueryParams.h"
#include "FGridVector.h"
#include "Basics.generated.h"

/**
 * 
 */
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
