#pragma once

#include "../nclgl/OGLRenderer.h"
#include <vector>
#include "../nclgl/Mesh.h"
#include "../nclgl/Matrix4.h"
#include "../nclgl/Vector3.h"
#include "../nclgl/Vector4.h"
#include "../nclgl/CubeRobot.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/Frustum.h"


class Camera;
class Mesh;
class HeightMap;
class Camera;
class Shader;
class MeshAnimation;
class MeshMaterial;
class SceneNode;



class Renderer : public OGLRenderer
{
public:
	Renderer(Window& parent);
	~Renderer(void);

	
	void UpdateScene(float dt) override;
	void RenderScene() override;
	


protected:
	void DrawShadowScene();
	void DrawMainScene(float dt);
	void DrawHeightMap();
	void DrawWater();
	void DrawSkybox();
	void DrawSoldier(float dt);
	void DrawTree(float dt);
	
	void BuildNodeLists(SceneNode* from);
	void SortNodeLists();
	void ClearNodeLists();
	void DrawNodes();
	void DrawNode(SceneNode* n);



	GLuint shadowTex;
	GLuint shadowFBO;
	GLuint cubeMap;
	GLuint waterTex;
	GLuint earthTex;
	GLuint earth;
	GLuint earthBump;
	GLuint sceneDiffuse;
	GLuint sceneBump;
	GLuint sun1;
	GLuint sun2;
	GLuint sun3;
	GLuint sun4;
	GLuint glass;
	GLuint tree;
	GLuint treeRoot;
	GLuint texture;
	


	bool isFollowingEnabled = false;
	bool isTrackEnabled = false;

	int currentFrame;

	float frameTime;
	float sceneTime;
	float waterRotate;
	float waterCycle;
	float soldierX;
	float soldierZ;

	Shader* basicShader;
	Shader* sceneShader;
	Shader* shadowShader;
	Shader* lightShader;
	Shader* reflectShader;
	Shader* skyboxShader;
	Shader* soldierShader;
	Shader* glassShader;
	Shader* treeShader;
	Shader* shader;
	Shader* pointLightShader;
	Shader* combineShader;

	HeightMap* heightMap;
	Mesh* quad;
	Mesh* soldier;
	Mesh* cone;
	Mesh* cylinder;

	MeshAnimation* anim;
	MeshMaterial* material;

	vector<GLuint> matTextures;
	vector<Mesh*> sceneMeshes;
	vector<Matrix4> sceneTransforms;
	
	vector<Mesh*> treeMeshs;
	vector<Matrix4> treeTransforms;

	vector<SceneNode*> transparentNodeList;
	vector<SceneNode*> nodeList;


	Camera* camera;
	Light* light;
	Light* mapLight;
	Mesh* cube;

	Frustum frameFrustum;
	
	SceneNode* cubenode;
	SceneNode* root;
	SceneNode* rain;



};