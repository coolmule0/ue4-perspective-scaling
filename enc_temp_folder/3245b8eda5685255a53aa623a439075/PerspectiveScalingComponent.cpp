// Fill out your copyright notice in the Description page of Project Settings.

#include "PerspectiveScalingComponent.h"

#include "Engine/World.h" 
#include "Engine/EngineTypes.h" 
#include "Engine/StaticMeshActor.h" 
#include "Templates/Casts.h"
#include "Components/PrimitiveComponent.h"
#include "Components/StaticMeshComponent.h" 
#include "DrawDebugHelpers.h"
#include "Engine/StaticMesh.h" 
#include "Math/TransformNonVectorized.h"
#include "Engine/Engine.h" 

#define TRACE_PERSPECTIVESCALE		ECC_GameTraceChannel3

// Sets default values for this component's properties
UPerspectiveScalingComponent::UPerspectiveScalingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	GrabDistance = 3000.f;
	DrawDebugTraceLines = false;
	DistanceLeeway = 50.f;
}


// Called when the game starts
void UPerspectiveScalingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}

void UPerspectiveScalingComponent::TraceForObject()
{
	const FVector TraceStart = this->GetComponentLocation();
	const FVector TraceEnd = TraceStart + (this->GetForwardVector() * GrabDistance);
	FHitResult ResultingHit = FHitResult(ForceInit);

	if (DrawDebugTraceLines) {
		DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, true);
	}

	if (GetWorld()->LineTraceSingleByChannel(ResultingHit, TraceStart, TraceEnd, TRACE_PERSPECTIVESCALE)) {
		if (ResultingHit.bBlockingHit) {
			AActor* foo = ResultingHit.GetActor();
			PickupObject(*foo);
		}
	}
}

float UPerspectiveScalingComponent::FindPerspectivePoint() const
{
	float ClosestTraceHit = GrabDistance;
	FVector ActorOrigin, ActorBounds;
	AttachedActor->GetActorBounds(true, ActorOrigin, ActorBounds);

	TArray<FVector> TraceVertices;

	//calculate cube vertices
	FVector StartVert = ActorOrigin - ActorBounds;
	FVector FullVect = 2 * ActorBounds;
	TraceVertices.Emplace(StartVert);
	TraceVertices.Emplace(StartVert + FVector(FullVect.X,	0,			0));
	TraceVertices.Emplace(StartVert + FVector(0,			FullVect.Y,	0));
	TraceVertices.Emplace(StartVert + FVector(0,			0,			FullVect.Z));
	TraceVertices.Emplace(StartVert + FVector(FullVect.X,	FullVect.Y,	0));
	TraceVertices.Emplace(StartVert + FVector(FullVect.X,	0,			FullVect.Z));
	TraceVertices.Emplace(StartVert + FVector(0,			FullVect.Y, FullVect.Z));
	TraceVertices.Emplace(StartVert + FullVect);

	FCollisionQueryParams Params = FCollisionQueryParams::DefaultQueryParam;
	Params.AddIgnoredActor(AttachedActor);
	const FVector TraceStart = this->GetComponentLocation();

	for (auto& Vertex : TraceVertices)
	{
		const FVector TraceEnd = Vertex;
		FHitResult HitResult = FHitResult(ForceInit);

		if (DrawDebugTraceLines) {
			DrawDebugLine(GetWorld(), TraceStart, TraceEnd, FColor::Green, false);
		}
		if (GetWorld()->LineTraceSingleByChannel(HitResult, TraceStart, TraceEnd, ECC_Visibility, Params)) {
			ClosestTraceHit = (HitResult.Distance < ClosestTraceHit) ? HitResult.Distance : ClosestTraceHit;
		}
	}
	return ClosestTraceHit;
}

// link up the object with owner
void UPerspectiveScalingComponent::PickupObject(AActor& ObjectToAttach)
{
	AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(&ObjectToAttach);
	bHoldingObject = true;
	AttachedActor = &ObjectToAttach;
	FAttachmentTransformRules AttachmentRules = FAttachmentTransformRules::KeepWorldTransform;
	AttachmentRules.bWeldSimulatedBodies = true;
	ObjectToAttach.AttachToComponent(this->GetAttachParent(), AttachmentRules);

	UStaticMeshComponent *SMC = StaticMeshActor->GetStaticMeshComponent();
	SMC->SetEnableGravity(false);
	SMC->SetSimulatePhysics(false);

}

void UPerspectiveScalingComponent::DropAttachedObject()
{
	AStaticMeshActor* StaticMeshActor = Cast<AStaticMeshActor>(AttachedActor);
	UStaticMeshComponent* SMC = StaticMeshActor->GetStaticMeshComponent();
	SMC->SetEnableGravity(true);
	SMC->SetSimulatePhysics(true);
	bHoldingObject = false;
	AttachedActor = NULL;

}


// Called every frame
void UPerspectiveScalingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
	
	//Scale and move attached object
	if (bHoldingObject) {
		FVector RelativeLocation = AttachedActor->GetActorLocation() - this->GetComponentLocation();
		float ClosestDistance = FindPerspectivePoint();
		float ScaleFactor = (ClosestDistance - DistanceLeeway) / RelativeLocation.Size();

		//Adjest actor values
		AttachedActor->SetActorScale3D(AttachedActor->GetActorScale3D() * ScaleFactor);
		AttachedActor->SetActorLocation(RelativeLocation * ScaleFactor + this->GetComponentLocation());
		UStaticMeshComponent* StaticMeshActorComponent = (Cast<AStaticMeshActor>(AttachedActor))->GetStaticMeshComponent();
		StaticMeshActorComponent->SetMassScale(NAME_None, StaticMeshActorComponent->GetMassScale() * ScaleFactor );
	}
}

void UPerspectiveScalingComponent::ToggleGrabbedObject()
{
	if (bHoldingObject) {
		DropAttachedObject();
	}
	else {
		TraceForObject();
	}
}

