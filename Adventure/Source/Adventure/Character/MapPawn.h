// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <deque>
#include "Basics.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MapPawn.generated.h"

USTRUCT(BlueprintType)
struct FCameraSettings
{
	GENERATED_BODY()

	//Units in Degrees
	UPROPERTY(BlueprintReadWrite)
	float AngularVelocity;

	//Units in m/s
	UPROPERTY(BlueprintReadWrite)
	float ZoomSpeed;

	UPROPERTY(BlueprintReadWrite)
	float MaxUpRotation;

	UPROPERTY(BlueprintReadWrite)
	float MaxDownRotation;

	UPROPERTY(BlueprintReadWrite)
	float MaxInZoom;

	UPROPERTY(BlueprintReadWrite)
	float MaxOutZoom;

	UPROPERTY(BlueprintReadWrite)
	float FinalCameraZoom;

};



UCLASS()
class ADVENTURE_API AMapPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMapPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	void ServerOnly_SetClassIndex(const int Index);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void ServerOnly_SetOwnerID(const int ID);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	int GetPawnID()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	int GetOwnerID()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	bool IsMoving()const;

	// Camera Controlls
	const FVector GetCameraLocation() const;
	void RotateCameraPitch(const float& AxisValue, const float& DeltaTime);
	void RotateCameraYaw(const float& AxisValue, const float& DeltaTime);
	void ZoomCamera(const float& AxisValue, const float& DeltaTime);

	// Server Only Functions
	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void ServerOnly_SetDestination(const FVector & Destination);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void ServerOnly_SetTargetLocation(const FVector& Location);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	FVector ServerOnly_GetDesiredForwardVector() const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void SetFocusToPawn(float TransitionTime);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	class UStatisticsComponent* GetStats()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	class UInventoryComponent* GetInventory()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	class UInteractionInterfaceComponent* GetInteractionInterface()const;

protected:

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	class UHoverArrowComponent* ArrowComponent;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	class USpringArmComponent* CameraBoom;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	class UCameraComponent* FollowCamera;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	class USceneComponent* Scene;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Statistics")
	class UStatisticsComponent* Statistics;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Inventory")
	class UInventoryComponent* Inventory;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "AttackInterface")
	class UInteractionInterfaceComponent* InteractionInterface;

private:

	bool bRotatePawn;
	bool bHasTarget;
	bool bWasPathingSucessfull;
	int m_ClassIndex;
	int m_CelebrationAnimIndex;
	int m_AttackAnimIndex;

	FVector m_FullDestination;
	FVector m_ForwardVector;
	FVector m_Destination;
	FVector m_TargetedLocation;
	FCameraSettings m_CameraSettings;
	std::deque<FVector> m_MoveQueue;
	class FPathFinder* m_CurrentRequest;

	// Private methods
	void RotatePawn(float DeltaTime);
	void MovePawn(float DeltaTime);
	void OnNewPathRecieved(bool success, TArray<FVector> Path);
	static int GetNewID();

	// Server Property/Functions
	UPROPERTY(Replicated)
	int m_PawnID;

	UPROPERTY(Replicated)
	int m_OwnerID;

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_SetActiveDestination(const FVector& Location);
};
