// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "MapPawnComponent_Head.generated.h"

UCLASS(ClassGroup = (Custom), meta = (BlueprintSpawnableComponent), Blueprintable)
class ADVENTURE_API UMapPawnComponent_Head : public UStaticMeshComponent
{
	GENERATED_BODY()
	
	public:	
	// Sets default values for this actor's properties
	UMapPawnComponent_Head();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

};
