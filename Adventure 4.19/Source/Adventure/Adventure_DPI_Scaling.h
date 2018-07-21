// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DPICustomScalingRule.h"
#include "Adventure_DPI_Scaling.generated.h"

/**
 * 
 */
UCLASS()
class ADVENTURE_API UAdventure_DPI_Scaling : public UDPICustomScalingRule
{
	GENERATED_BODY()
	
	
public:
	virtual float GetDPIScaleBasedOnSize(FIntPoint Size) const override;

	UPROPERTY(EditAnywhere, Category = "Resolution")
	float X = 1280;

	UPROPERTY(EditAnywhere, Category = "Resolution")
	float Y = 720;
	
};
