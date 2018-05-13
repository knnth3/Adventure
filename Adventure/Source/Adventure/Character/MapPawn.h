// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "MapPawn.generated.h"



USTRUCT(BlueprintType)
struct ADVENTURE_API FStatSheet
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	int Health = 10;

	//Meters per second
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	float MoveSpeed = 5;

};

UCLASS()
class ADVENTURE_API AMapPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMapPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	//Set the pawns destination location
	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	void SetDestination(FVector WorldLocation);

	//Returns the pawns stats
	UFUNCTION(BlueprintCallable, Category = "Map Pawn")
	FStatSheet GetStatSheet()const;

private:

	//Moves a pawn if its destination is not the same as its position
	void MovePawn(float DeltaTime);

	//Server Functions

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_SetDestination(FVector WorldLocation);

	UPROPERTY(Replicated)
	FStatSheet m_stats;

	UPROPERTY(Replicated)
	bool bMoving;

	UPROPERTY(Replicated)
	FVector m_destination;
};
