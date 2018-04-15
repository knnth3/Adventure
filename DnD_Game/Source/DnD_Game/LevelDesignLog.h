// Fill out your copyright notice in the Description page of Project Settings.

#pragma once
#include <vector>
//#include <json.hpp>

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LevelDesignLog.generated.h"

//using json = nlohmann::json;

USTRUCT(BlueprintType)
struct DND_GAME_API FDecorative
{
	GENERATED_USTRUCT_BODY()

public:
	FDecorative()
	{
		name = "";
		position = FVector(0,0,0);
	}

	FDecorative(FString Name, FVector Position)
	{
		name = Name;
		position = Position;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decorative")
	FString name;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Decorative")
	FVector position;
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
	bool SaveLogTo(FString location);

private:

	//json m_savedata;
	FString m_levelName;
	std::vector<FDecorative> m_placedObjects;
		
};
