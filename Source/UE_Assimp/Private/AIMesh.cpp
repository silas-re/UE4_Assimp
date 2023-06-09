// Fill out your copyright notice in the Description page of Project Settings.


#include "AIMesh.h"

#include "AIScene.h"
#include "KismetProceduralMeshLibrary.h"
#include "MeshDescriptionBuilder.h"
#include "StaticMeshDescription.h"
#include "UE_Assimp.h"

void UAIMesh::GetMeshVertices(TArray<FVector>& Vertices)
{
	if (!this)
	{
		UE_LOG(LogAssimp, Fatal, TEXT("No Mesh"));
		return;
	}
	if (!Mesh)
	{
		UE_LOG(LogAssimp, Fatal, TEXT("No Mesh Data"));
		return;
	}
	Vertices.Empty();
	Vertices.AddUninitialized(Mesh->mNumVertices);
	for (unsigned int Index = 0; Index < Mesh->mNumVertices; Index++)
	{
		Vertices[Index] = ToVector(Mesh->mVertices[Index]);
	}
}

void UAIMesh::GetMeshNormals(TArray<FVector>& Normals)
{
	if (!this)
	{
		UE_LOG(LogAssimp, Fatal, TEXT("No Mesh"));
		return;
	}
	if (!Mesh)
	{
		UE_LOG(LogAssimp, Fatal, TEXT("No Mesh Data"));
		return;
	}
	Normals.Empty();
	Normals.AddUninitialized(Mesh->mNumVertices);
	for (unsigned int Index = 0; Index < Mesh->mNumVertices; Index++)
	{
		Normals[Index] = ToVector(Mesh->mNormals[Index]);
	}
}

void UAIMesh::GetMeshDataForProceduralMesh(TArray<FVector>& Vertices, TArray<int32>& Triangles,
                                           TArray<FVector>& Normals, TArray<FVector2D>& UV0,
                                           TArray<FProcMeshTangent>& Tangents)
{
	if (!this)
	{
		UE_LOG(LogAssimp, Fatal, TEXT("No Mesh"));
		return;
	}
	if (!Mesh)
	{
		UE_LOG(LogAssimp, Fatal, TEXT("No Mesh Data"));
		return;
	}

	Tangents.Reset();
	Vertices.Reset();
	Triangles.Reset();
	Normals.Reset();
	UV0.Reset();

	Vertices.AddUninitialized(Mesh->mNumVertices);
	Normals.AddUninitialized(Mesh->mNumVertices);
	if (Mesh->HasTangentsAndBitangents())
	{
		Tangents.AddUninitialized(Mesh->mNumVertices);
	}
	else
	{
		//
		UE_LOG(LogAssimp, Warning, TEXT("Mesh is missing Tangents"));
	}
	UV0.AddUninitialized(Mesh->mNumVertices);


	for (unsigned int Index = 0; Index < Mesh->mNumVertices; Index++)
	{
		Normals[Index] = ToVector(Mesh->mNormals[Index]);
		Vertices[Index] = ToVector(Mesh->mVertices[Index]);

		if (Mesh->mTangents)
		{
			Tangents[Index].TangentX = ToVector(Mesh->mTangents[Index]);
		}

		if (Mesh->HasTextureCoords(0))
		{
			UV0[Index].X = Mesh->mTextureCoords[0][Index].x;
			UV0[Index].Y = Mesh->mTextureCoords[0][Index].y;
		}
	}


	for (unsigned int i = 0; i < Mesh->mNumFaces; i++)
	{
		const auto Face = Mesh->mFaces[i];
		for (unsigned index = 0; index < Face.mNumIndices; index++)
		{
			Triangles.Push(Face.mIndices[index]);
		}
	}
}

UStaticMesh* UAIMesh::GetStaticMesh()
{
	if (StaticMesh) {
		return StaticMesh;
	}
	MeshDescription = UStaticMesh::CreateStaticMeshDescription(this);

	FMeshDescriptionBuilder MeshDescBuilder;

	MeshDescBuilder.SetMeshDescription(&MeshDescription->GetMeshDescription());
	MeshDescBuilder.EnablePolyGroups();
	int uvLayers = 0;
	//while (Mesh->HasTextureCoords(uvLayers)) {
	//	uvLayers++;
	//}

	MeshDescBuilder.SetNumUVLayers(uvLayers+1);

	UE_LOG(LogAssimp, Warning, TEXT("building mesh"))

	TArray<FVertexInstanceID> VertexInstances;
	VertexInstances.AddUninitialized(Mesh->mNumVertices);
	TArray<FUVID> UVIds;
	UVIds.AddUninitialized(Mesh->mNumVertices);

	for (unsigned int Index = 0; Index < Mesh->mNumVertices; Index++)
	{
		auto vec = ToVector(Mesh->mVertices[Index]);
		vec.Z = -vec.Z;
		auto tmpX = vec.X;
		vec.X = vec.Y;
		vec.Y = tmpX;
		auto VertexID = MeshDescBuilder.AppendVertex(vec);

		auto Instance = MeshDescBuilder.AppendInstance(VertexID);
		VertexInstances[Index] = Instance;
		if(Mesh->HasNormals())
		{
			auto nrm = ToVector(Mesh->mNormals[Index]);
			nrm.Z = -nrm.Z;
			auto tmpXN = nrm.X;
			nrm.X = nrm.Y;
			nrm.Y = tmpXN;
			MeshDescBuilder.SetInstanceNormal(Instance, nrm);
			// let unreal build its own normals
		}else
		{
			UE_LOG(LogAssimp,Warning,TEXT("Normals not found consider generating them with assimp"))
		}

		int currUvLayer = 0;

		if (Mesh->HasTextureCoords(currUvLayer))
		{
			auto UvId = MeshDescBuilder.AppendUV(
				FVector2D(
					Mesh->mTextureCoords[currUvLayer][Index].x, 
					-Mesh->mTextureCoords[currUvLayer][Index].y),
				0);
			UVIds[Index] = UvId;
		}
	}

	const FPolygonGroupID PolygonGroup = MeshDescBuilder.AppendPolygonGroup();


	for (unsigned int i = 0; i < Mesh->mNumFaces; i++)
	{
		const auto Face = Mesh->mFaces[i];
		if(Face.mNumIndices>2)
		{
			auto triId = MeshDescBuilder.AppendTriangle(VertexInstances[Face.mIndices[0]], VertexInstances[Face.mIndices[1]],
										   VertexInstances[Face.mIndices[2]], PolygonGroup);
			if (Mesh->HasTextureCoords(0)) {
				MeshDescBuilder.AppendUVTriangle(triId, UVIds[Face.mIndices[0]], UVIds[Face.mIndices[1]], UVIds[Face.mIndices[2]], 0);
			}
		}
	}
	// At least one material must be added
	StaticMesh = NewObject<UStaticMesh>(this);
	StaticMesh->GetStaticMaterials().Add(FStaticMaterial());

	UStaticMesh::FBuildMeshDescriptionsParams MeshDescriptionsParams;
	MeshDescriptionsParams.bBuildSimpleCollision = true;
	


	// Build static mesh
	TArray<const FMeshDescription*> MeshDescriptions;
	MeshDescriptions.Emplace(&MeshDescription->GetMeshDescription());
	StaticMesh->BuildFromMeshDescriptions(MeshDescriptions, MeshDescriptionsParams);


	return StaticMesh;
}


int UAIMesh::GetNumVertices()
{
	return Mesh->mNumVertices;
}

void UAIMesh::GetAllBones(TArray<FAIBone>& Bones)
{
	for (unsigned int i = 0; i < Mesh->mNumBones; i++)
	{
		Bones.Add(FAIBone(Mesh->mBones[i]));
	}
}

FString UAIMesh::GetMeshName() const
{
	return UTF8_TO_TCHAR(Mesh->mName.C_Str());
}

int UAIMesh::GetMaterialIndex()
{
	return Mesh->mMaterialIndex;
}

