// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "PerspectiveScalingComponent.generated.h"

/**
* Attach to player camera. Allows grabbing and resizing of objects without changing the size as seen from the camera
*/
UCLASS( ClassGroup=(PerspectiveScaling), meta=(BlueprintSpawnableComponent) )
class PERSPECTIVESCALING_API UPerspectiveScalingComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPerspectiveScalingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	// Trace cast for a valid object
	void TraceForObject();
	// Trace actor vertices to find closest point
	float FindPerspectivePoint() const;

	void PickupObject(AActor& ObjectToAttach);
	void DropAttachedObject();

	// The "grabbed" actor
	bool bHoldingObject;
	AActor* AttachedActor;


public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void ToggleGrabbedObject();

	// How far away from origin to look for objects to grab and how far away grabbed objects can move
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float GrabDistance;

	// Draw debug lines for when tracing occurs
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool DrawDebugTraceLines;

	// Moving the actor will avoid other objects by this amount
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float DistanceLeeway;

};
