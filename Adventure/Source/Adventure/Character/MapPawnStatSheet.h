// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "Engine/UserDefinedStruct.h"
#include "MapPawnStatSheet.generated.h"

/**
 * 
 */

UENUM()
enum class PAWN_CLASS_TYPE : uint8
{
	FIGHTER,
	WIZARD,
	SORCERER,
	BARD
};

UENUM()
enum class ATTACK_ELEMENT_TYPE : uint8
{
	PHYSICAL,
	ICE,
	FIRE,
	LIGHTING,
	WATER
};


USTRUCT(BlueprintType)
struct ADVENTURE_API FMapPawnStatSheet
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

	// Base Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	PAWN_CLASS_TYPE Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	int MaxHealth = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	int CurrentHealth = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	int CurrentLevel = 1;

	// Ability Stats
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	int Strength;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	int Dexterity;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	int Constitution;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	int Intelligence;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	int Wisdom;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Stat Sheet")
	int Charisma;
};

UCLASS()
class AMapPawnAttack : public AActor
{
	GENERATED_BODY()
public:

	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	ATTACK_ELEMENT_TYPE ElementType;

	UFUNCTION(BlueprintNativeEvent)
	void Attack(const FMapPawnStatSheet& Stats, const FVector& StartLocation, const FVector& EndLocation) const;
	virtual void Attack_Implementation(const FMapPawnStatSheet& Stats, const FVector& StartLocation, const FVector& EndLocation) const;
};

UCLASS()
class AMapPawnAttack_Basic : public AMapPawnAttack
{
	GENERATED_BODY()
	
public:
	virtual void Attack_Implementation(const FMapPawnStatSheet& Stats, const FVector& StartLocation, const FVector& EndLocation) const override;
};