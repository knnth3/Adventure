// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include <queue>
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

};

USTRUCT(BlueprintType)
struct ADVENTURE_API FStatSheet
{
	GENERATED_BODY()
	
	//Rotate Speed
	UPROPERTY()
	float TurnSpeed = 15.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	FString Name = "Alfie";

	//Units in feet (walking)
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	float MoveSpeed = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	int Health = 10;

};

UCLASS()
class ADVENTURE_API AMapPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMapPawn();

	UFUNCTION(BlueprintNativeEvent, Category = "MapPawn Camera")
	void RotateUpDown(const float& AxisValue, const float& DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category = "MapPawn Camera")
	void RotateLeftRight(const float& AxisValue, const float& DeltaTime);

	UFUNCTION(BlueprintNativeEvent, Category = "MapPawn Camera")
	void ZoomInOut(const float& AxisValue, const float& DeltaTime);

protected:

	/** Units in meters/seconds/degrees */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	FCameraSettings CameraSettings;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USceneComponent* Scene;

	/** Camera boom positioning the camera behind the character */
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USpringArmComponent* CameraBoom;

	/** Follow camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Camera")
	class UCameraComponent* FollowCamera;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	class USkeletalMeshComponent* PawnBody;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when the pawn is about to be destroyed
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Set the pawns destination location
	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void SetDestination(FGridCoordinate GridLocation);

	//Returns the pawns stats
	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	FStatSheet GetStatSheet()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	bool IsMoving()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	int GetOwnerID()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void SetOwnerID(const int ID);

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	int GetPawnID()const;

	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void SetPawnID(const int ID);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Map Pawn")
	void SetBodyArmor(const int Index);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Map Pawn")
	void SetHead(const int Index, const bool bBoy);

protected:
	//BP Overridable functions

	UFUNCTION(BlueprintImplementableEvent, Category = "Map Pawn")
	void BeginMove();

	UFUNCTION(BlueprintImplementableEvent, Category = "Map Pawn")
	void EndMove();

private:

	//Moves a pawn if its destination is not the same as its position
	void MovePawn(float DeltaTime);

	//Rotates a pawn
	void RotatePawn(float DeltaTime);

	//Get a delta position given the time and speed from pawn stats
	FVector GetNextMove(float DeltaTime, bool& bHasNextMove);

	//Gets the current position in grid space
	FGridCoordinate GetActorGridLocation()const;

	//Checks against the WorldGrid to see if a move is valid
	bool IsMoveValid(FVector DeltaLocation)const;

	void MoveToNextLocation();

	//Server Functions

	UFUNCTION()
	void OnDestination_Rep();

	UFUNCTION()
	void OnRotation_Rep();

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetDestination(FGridCoordinate GridLocation, bool bRotateOnly = false);

	UPROPERTY(Replicated)
	int OwnerID;

	UPROPERTY(Replicated)
	int PawnID;

	UPROPERTY(Replicated)
	FStatSheet CharacterStats;

	UPROPERTY(ReplicatedUsing = OnDestination_Rep)
	FVector FinalDestination;

	UPROPERTY(ReplicatedUsing = OnRotation_Rep)
	FRotator Rotation;

	int SkeletalMeshIndex;
	bool bMoveCharacter;
	bool bRotateCharacter;

	std::queue<FGridCoordinate> MoveQueue;
};
