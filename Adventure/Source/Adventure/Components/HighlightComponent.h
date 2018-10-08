// By: Eric Marquez. All information and code provided is free to use and can be used comercially.Use of such examples indicates no fault to the author for any damages caused by them. The author must be credited.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "HighlightComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ADVENTURE_API UHighlightComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UHighlightComponent();

	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Highlight Component")
	void AddHighlightableComponents(TArray<UPrimitiveComponent*> Components);

	UFUNCTION(BlueprintCallable, Category = "Highlight Component")
	void HighlightObjects();

	UFUNCTION(BlueprintCallable, Category = "Highlight Component")
	void DimObjects();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

private:	

	TArray<UPrimitiveComponent*> m_HighlightableComponents;
		
	
};
