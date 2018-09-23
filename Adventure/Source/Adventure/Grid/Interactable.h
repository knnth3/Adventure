// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

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
	virtual void Tick(float DeltaTime) override;
	void SetStaticMesh(class UStaticMesh* StaticMesh);

	UFUNCTION(BlueprintCallable)
	void MakeTransparent();

protected:

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	class USceneComponent* Scene;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	class UStaticMeshComponent* Visual;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	class UBoxComponent* BoxCollision;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Visual")
	bool bCanBeTransparent;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UFUNCTION(BlueprintImplementableEvent, Category = "Interactable")
	void OnVisibilityValueChanged(const float Value);
	
private:

	float VisibilityValue;
	float TargetVisibility;
	float OpacityTransitionValue;
};
