// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <vector>
#include <map>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MapSaveParser.h"
#include "LevelDesignLog.generated.h"

struct SaveObject
{
	SaveObject()
	{
		name = ShortString();
		x = 0;
		y = 0;
	}
	SaveObject(FString Name, float X, float Y)
	{
		name = Name;
		x = X;
		y = Y;
	}

	ShortString name;
	float x;
	float y;
};

USTRUCT(BlueprintType)
struct DND_GAME_API FDecorative
{
	GENERATED_USTRUCT_BODY()

public:
	FDecorative()
	{
		name = "";
		position = FVector2D(0,0);
	}

	FDecorative(FString Name, FVector2D Position)
	{
		name = Name;
		position = Position;
	}

	FDecorative(SaveObject obj)
	{
		name = FString(obj.name.to_fstr());
		position.X = obj.x;
		position.Y = obj.y;
	}

	SaveObject GetSaveObject()const
	{
		return SaveObject(name, position.X, position.Y);
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decorative")
	FString name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decorative")
	FVector2D position;
};

UCLASS()
class DND_GAME_API ALevelDesignLog : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	ALevelDesignLog();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual void AddNewObject(FDecorative newItem);

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual void SetLevelName(FString name);

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual void SetLevelSize(int rows, int columns);

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual void SetLevelInfo(FString name, int rows, int columns);

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual bool SaveLogTo(FString location);

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual bool LoadLogFrom(FString location);

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual bool IsGridSpaceOccupied(FVector2D position) const;

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual int GetActorCount() const;

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual UStaticMesh* GetMeshOf(int index) const;

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual bool GetInfoOf(int index, FDecorative& obj) const;

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual void GetLevelInfo(FString& name, int& rows, int& columns) const;

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual void GetLevelSize(int& rows, int& columns) const;

	UFUNCTION(BlueprintCallable, Category = "LevelDesign")
	virtual void GetLevelName(FString& name) const;

	//UClass* DefaultPawnClass = FindObject<UClass>(ANY_PACKAGE, *pawnClassName);

private:

	FString m_levelName;
	int m_mapRows;
	int m_mapColumns;
	std::vector<FDecorative> m_placedObjects;
	std::map<std::pair<int, int>, bool> m_occupiedGridSpaces;
		
};
