#ifndef LOAD_MESH_H
#define LOAD_MESH_H

#include <string>
#include <GL/glew.h>
#include <assimp/Scene.h>

namespace assets {

struct MeshData {
	
	unsigned int mVao;
	unsigned int mVboVerts;
	unsigned int mVboNormals;
	unsigned int mVboTexCoords;
	unsigned int mIndexBuffer;
	float mScaleFactor; //TODO replace with bounding box
	unsigned int mNumIndices;
	const aiScene* mScene;
	aiVector3D mBbMin, mBbMax;

	MeshData::MeshData() : mVao(-1), mVboVerts(-1), mVboNormals(-1), mVboTexCoords(-1), 
		                   mIndexBuffer(-1), mScaleFactor(0.0f), mNumIndices(0), mScene(NULL) {
	}
};

void BufferIndexedVerts(MeshData& meshdata);

MeshData LoadMesh(const std::string& pFile);

void GetBoundingBox(const aiScene* scene, aiVector3D* min, aiVector3D* max);

void GetBoundingBox(const aiMesh* mesh, aiVector3D* min, aiVector3D* max);

}

#endif