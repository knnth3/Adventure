// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "HoverArrowComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent), Blueprintable)
class ADVENTURE_API UHoverArrowComponent : public UStaticMeshComponent
{
	GENERATED_BODY()

public:	

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Settings")
	float Amplitude;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Hover Settings")
	float Frequency;

	// Sets default values for this component's properties
	UHoverArrowComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Hover Arrow Component")
	void ShowCursor();

	UFUNCTION(BlueprintCallable, Category = "Hover Arrow Component")
	void HideCursor();

protected:

	// Called when the game starts
	virtual void BeginPlay() override;

private:

	bool bActive;
	float ElapsedTime;
	float BaseHeight;
};
