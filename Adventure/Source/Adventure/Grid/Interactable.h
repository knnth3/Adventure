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
#include "GameFramework/Actor.h"
#include "Interactable.generated.h"

UCLASS()
class ADVENTURE_API AInteractable : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractable();
	void SetStaticMesh(class UStaticMesh* StaticMesh);
	void ServerOnly_SetObjectID(int ID);
	void ServerOnly_SetClassIndex(int Index);

	int GetClassIndex_Implementation() const;
	int GetObjectID_Implementation() const;
	bool IsNonTraversable_Implementation() const;
	bool IsBlockingSpace_Implementation() const;

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USceneComponent* Scene;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	class UStaticMeshComponent* Visual;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	class UBoxComponent* BoxCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bCanBeTransparent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grid Logic")
	bool bIsTraversable;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Grid Logic")
	bool bHasMovementPenalty;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
private:

	int m_ClassIndex;
	int m_ObjectID;
};
