// Fill out your copyright notice in the Description page of Project Settings.

#include "GI_Adventure.h"
#include "UObject/ConstructorHelpers.h"
#include "Blueprint/UserWidget.h"

UGI_Adventure::UGI_Adventure()
{
	static ConstructorHelpers::FClassFinder<UUserWidget> ptr(TEXT("/Game/Blueprints/UI/MainMenu/MainMenu"));

	//Log error om fail
	if (!ensure(ptr.Class != nullptr)) return;

	//Syntax for console logging
	UE_LOG(LogTemp, Warning, TEXT("Found Class %s"), *ptr.Class->GetName());

	m_UMGBlueprint = ptr.Class;
}

void UGI_Adventure::LoadMainMenu()
{
	if (!ensure(m_UMGBlueprint != nullptr)) return;

	UUserWidget* Menu = CreateWidget<UUserWidget>(this, m_UMGBlueprint);

	if (!ensure(Menu != nullptr)) return;

	Menu->AddToViewport();
}
