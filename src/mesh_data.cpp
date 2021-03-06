#include "mesh_data.hpp"

#include <stdexcept>

// Assimp
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/Importer.hpp>

using rt::mesh_data;

rt::mesh_data::mesh_data(const std::string &path)
{
	Assimp::Importer importer;
	const aiScene *scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		throw std::runtime_error("assimp import error (could not open file?)");
	}

	process_assimp_node(scene, scene->mRootNode);
}

mesh_data::mesh_data(const aiMesh *mesh)
{
	append_assimp_mesh(mesh);
}

void mesh_data::append_assimp_mesh(const aiMesh *mesh)
{
	std::vector<glm::vec3> positions;
	std::vector<glm::vec3> normals;
	std::vector<glm::vec2> uvs;

	bool has_uvs = mesh->mTextureCoords[0];

	for (unsigned int i = 0; i < mesh->mNumVertices; i++)
	{
		positions.emplace_back(
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z);

		normals.emplace_back(
			mesh->mNormals[i].x,
			mesh->mNormals[i].y,
			mesh->mNormals[i].z);
	
		if (has_uvs)
		{
			uvs.emplace_back(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y);
		}
	}

	// Check if we have equal number of everything
	if (positions.size() != normals.size() || (uvs.size() > 0 && uvs.size() != positions.size()))
		throw std::runtime_error("attempted to load a mesh with missing normals or UVs");

	// Make triangles (faces)
	for (unsigned int i = 0; i < mesh->mNumFaces; i++)
	{
		const auto &f = mesh->mFaces[i];
		rt::triangle t;

		t.vertices[0] = positions[f.mIndices[0]];
		t.vertices[1] = positions[f.mIndices[1]];
		t.vertices[2] = positions[f.mIndices[2]];

		t.normals[0] = normals[f.mIndices[0]];
		t.normals[1] = normals[f.mIndices[1]];
		t.normals[2] = normals[f.mIndices[2]];

		if (has_uvs)
		{
			t.uvs[0] = uvs[f.mIndices[0]];
			t.uvs[1] = uvs[f.mIndices[1]];
			t.uvs[2] = uvs[f.mIndices[2]];
		}

		t.material = nullptr;

		m_triangles.push_back(t);
	}
}

void mesh_data::process_assimp_node(const aiScene *scene, aiNode *node)
{
	// Process all meshes in this node
	for (unsigned int i = 0; i < node->mNumMeshes; i++)
		append_assimp_mesh(scene->mMeshes[node->mMeshes[i]]);
	
	// Process children nodes
	for (unsigned int i = 0; i < node->mNumChildren; i++)
		process_assimp_node(scene, node->mChildren[i]);
}
