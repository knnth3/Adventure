// Copyright 2019 Eric Marquez
// 
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
// 
// http ://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

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
