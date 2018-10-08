// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "Grid/GridEntity.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interactable.generated.h"

UCLASS()
class ADVENTURE_API AInteractable : public AActor, public IGridEntity
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AInteractable();
	void SetStaticMesh(class UStaticMesh* StaticMesh);
	void ServerOnly_SetObjectID(int ID);
	void ServerOnly_SetClassIndex(int Index);

	virtual int GetClassIndex_Implementation() const override;
	virtual int GetObjectID_Implementation() const override;
	virtual bool IsNonTraversable_Implementation() const override;
	virtual bool IsBlockingSpace_Implementation() const override;

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
