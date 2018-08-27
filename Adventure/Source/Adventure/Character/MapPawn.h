// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <queue>
#include "Basics.h"
#include "MapPawnStatSheet.h"
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

};

UCLASS()
class ADVENTURE_API AMapPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMapPawn();
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	// Owner Properties
	int GetOwnerID()const;
	void SetOwnerID(const int ID);
	int GetPawnID()const;
	void SetPawnID(const int ID);

	// Camera Controlls
	void RotateCameraPitch(const float& AxisValue, const float& DeltaTime);
	void RotateCameraYaw(const float& AxisValue, const float& DeltaTime);
	void ZoomCamera(const float& AxisValue, const float& DeltaTime);

	// Pawn Stats
	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	FMapPawnStatSheet GetStatSheet()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	bool IsMoving()const;

	// Pawn Actions
	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void SetDestination(FGridCoordinate GridLocation);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void Attack(const AMapPawnAttack* AttackMove, const FVector Location);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void SetAsActivePawn(bool Active);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void ScaleHead(const FVector& Scale);


protected:

	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	USkeletalMesh* BaseSkeletalMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	UStaticMesh* BaseHeadMesh;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	UAnimBlueprint* BaseAnimationBlueprint;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	TSubclassOf<class AMapPawnComponent_Head> PawnHeadClass;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "Skeletal Mesh")
	class USpringArmComponent* CameraBoom;

	// BP Overridable functions
	UFUNCTION(BlueprintImplementableEvent, Category = "Map Pawn")
	void OnBeginTurn();

	UFUNCTION(BlueprintImplementableEvent, Category = "Map Pawn")
	void OnEndTurn();


private:
	int SkeletalMeshIndex;
	bool bMoveCharacter;
	bool bRotateCharacter;
	bool bIsTurnActive;
	float DesiredCameraZoom;
	FCameraSettings CameraSettings;
	class USceneComponent* Scene;
	class UCameraComponent* FollowCamera;
	class USkeletalMeshComponent* PawnBody;
	class UStaticMeshComponent* PawnHeadC;
	class AMapPawnComponent_Head* PawnHead;
	std::queue<FGridCoordinate> MoveQueue;

	void MovePawn(float DeltaTime);
	void RotatePawn(float DeltaTime);
	FVector GetNextMove(float DeltaTime, bool& bHasNextMove);
	FGridCoordinate GetActorGridLocation()const;
	bool IsMoveValid(FVector DeltaLocation)const;
	void MoveToNextLocation();

	// Server Property/Functions

	UPROPERTY(Replicated)
	int OwnerID;

	UPROPERTY(Replicated)
	int PawnID;

	UPROPERTY(Replicated)
	FMapPawnStatSheet StatSheet;

	UPROPERTY(ReplicatedUsing = OnDestination_Rep)
	FVector FinalDestination;

	UPROPERTY(ReplicatedUsing = OnRotation_Rep)
	FRotator Rotation;

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetDestination(FGridCoordinate GridLocation, bool bRotateOnly = false);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_Attack(FVector Location);

	UFUNCTION()
	void OnDestination_Rep();

	UFUNCTION()
	void OnRotation_Rep();
};
