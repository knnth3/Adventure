// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FGridVector.generated.h"

//Works exactly like F2DVector but it more lightweight and uses integer math
USTRUCT(BlueprintType)
struct ADVENTURE_API FGridVector
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FGridVector")
	int X;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "FGridVector")
	int Y;
};