#include "Renderer.h"

#include <cmath>
#include <algorithm>
#include "../nclgl/Light.h"
#include "../nclgl/Camera.h"
#include "../nclgl/HeightMap.h"
#include "../nclgl/Shader.h"
#include "../nclgl/MeshAnimation.h"
#include "../nclgl/MeshMaterial.h"
#include "../nclgl/SceneNode.h"
#include "../nclgl/CubeRobot.h"


#define SHADOWSIZE 2048

Renderer::Renderer(Window& parent) : OGLRenderer(parent)
{

	quad = Mesh::GenerateQuad();
	cube = Mesh::LoadFromMeshFile("OffsetCubeY.msh");
	

	sceneDiffuse = SOIL_load_OGL_texture(TEXTUREDIR"Barren Reds.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sceneBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	//sun
	sun1 = SOIL_load_OGL_texture(TEXTUREDIR"SUN1.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sun2 = SOIL_load_OGL_texture(TEXTUREDIR"SUN2.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sun3 = SOIL_load_OGL_texture(TEXTUREDIR"SUN3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	sun4 = SOIL_load_OGL_texture(TEXTUREDIR"SUN4.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	glass = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	tree = SOIL_load_OGL_texture(TEXTUREDIR"tree.jpeg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	treeRoot = SOIL_load_OGL_texture(TEXTUREDIR"treeRoot.jpeg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	//heightMap
	heightMap = new HeightMap(TEXTUREDIR"noise2.bmp");
	waterTex = SOIL_load_OGL_texture(TEXTUREDIR"water.TGA", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthTex = SOIL_load_OGL_texture(TEXTUREDIR"red-wood.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	earthBump = SOIL_load_OGL_texture(TEXTUREDIR"Barren RedsDOT3.JPG", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);
	texture = SOIL_load_OGL_texture(TEXTUREDIR"stainedglass.tga", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, 0);
	cubeMap = SOIL_load_OGL_cubemap(TEXTUREDIR"3.jpg", TEXTUREDIR"3.jpg", TEXTUREDIR"3.jpg", TEXTUREDIR"3.jpg", TEXTUREDIR"3.jpg", TEXTUREDIR"3.jpg", SOIL_LOAD_RGB, SOIL_CREATE_NEW_ID, 0);
	earth = SOIL_load_OGL_texture(TEXTUREDIR"earth.jpeg", SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS);

	if (!earthTex || !earthBump || !cubeMap || !waterTex)
		return;
	SetTextureRepeating(cubeMap, true);
	SetTextureRepeating(sceneDiffuse, true);
	SetTextureRepeating(sceneBump, true);
	SetTextureRepeating(earthTex, true);
	SetTextureRepeating(earthBump, true);
	SetTextureRepeating(waterTex, true);
	SetTextureRepeating(sun1, true);
	SetTextureRepeating(sun2, true);
	SetTextureRepeating(sun3, true);
	SetTextureRepeating(sun4, true);

	shader = new Shader("SceneVertex.glsl", "SceneFragment.glsl");
	sceneShader = new Shader("ShadowSceneVertex.glsl", "ShadowSceneFragment.glsl");
	shadowShader = new Shader("ShadowVertex.glsl", "ShadowFragment.glsl");
	reflectShader = new Shader("ReflectVertex.glsl", "ReflectFragment.glsl");
	skyboxShader = new Shader("SkyboxVertex.glsl", "SkyboxFragment.glsl");
	lightShader = new Shader("PerPixelVertex.glsl", "PerPixelFragment.glsl");
	basicShader = new Shader("ShadowSceneVertex.glsl", "ShadowSceneFragment.glsl");
	soldierShader = new Shader("SkinningVertex.glsl", "TexturedFragment.glsl");
	glassShader = new Shader("ShadowSceneVertex.glsl", "ShadowSceneFragment.glsl");
	treeShader = new Shader("ShadowVertex.glsl", "ShadowFragment.glsl");
	pointLightShader = new Shader("PointLightVertex.glsl", "PointLightFragment.glsl");

	if (!sceneShader->LoadSuccess() || !shadowShader->LoadSuccess()|| !reflectShader->LoadSuccess() || !skyboxShader->LoadSuccess() || !lightShader->LoadSuccess())
		return;

	Vector3 heightMapSize = heightMap->GetHeightMapSize();
	//camera = new Camera(-40.0f, 320.0f, 0.0f, Vector3(-38.0f, 45.0f, 50.0f));
	camera = new Camera(-25.0f, 0.0f, 0.0f, heightMapSize * Vector3(0.5f, 3.0f, 0.5f));
	light = new Light(Vector3(1, 2500, 1), Vector4(5, 5, 5, 2), 15000.0f);
	mapLight = new Light(heightMapSize * Vector3(0.0f, 1.5f, 0.0f), Vector4(1, 0.8, 0, 1), heightMapSize.x);
	

	//sceneNode
	root = new SceneNode();

	for (int i = 0; i < 500; ++i)
	{
		SceneNode* s = new SceneNode();
		s->SetColour(Vector4(rand()%100*0.01, rand() % 100 * 0.01, rand() % 100 * 0.01, rand() % 100 * 0.01));
		s->SetTransform(Matrix4::Translation(Vector3(rand() % (int)heightMapSize.x, rand()%5000+500, rand() % (int)heightMapSize.z)));
		s->SetModelScale(Vector3(rand()%50, rand() % 50, rand() % 50));
		s->SetBoundingRadius(200.0f);
		s->SetMesh(cube);
		
		root->AddChild(s);
	}
	
	

	



	//shadow
	glGenTextures(1, &shadowTex);
	glBindTexture(GL_TEXTURE_2D, shadowTex);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWSIZE, SHADOWSIZE, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

	glBindTexture(GL_TEXTURE_2D, 0);

	glGenFramebuffers(1, &shadowFBO);
	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTex, 0);
	glDrawBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);

	glClear(GL_DEPTH_BUFFER_BIT);
	glViewport(0, 0, SHADOWSIZE, SHADOWSIZE);
	glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

	//mainScene
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Role_T.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Sphere.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Sphere.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Cube.msh"));
	sceneMeshes.emplace_back(Mesh::LoadFromMeshFile("Sphere.msh"));
	
	//soldier
	soldier = Mesh::LoadFromMeshFile("Role_T.msh");
	anim = new MeshAnimation("Role_T.anm");
	material = new MeshMaterial("Role_T.mat");

	for (int i = 0; i < soldier->GetSubMeshCount(); ++i)
	{
		const MeshMaterialEntry* matEntry = material->GetMaterialForLayer(i);

		const string* filename = nullptr;
		matEntry->GetEntry("Diffuse", &filename);
		string path = TEXTUREDIR + *filename;
		GLuint texID = SOIL_load_OGL_texture(path.c_str(), SOIL_LOAD_AUTO, SOIL_CREATE_NEW_ID, SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);
		matTextures.emplace_back(texID);
	}

	//tree
	treeMeshs.emplace_back(Mesh::LoadFromMeshFile("Cylinder.msh"));
	treeMeshs.emplace_back(Mesh::LoadFromMeshFile("Cone.msh"));
	treeMeshs.emplace_back(Mesh::LoadFromMeshFile("Cone.msh"));
	treeMeshs.emplace_back(Mesh::LoadFromMeshFile("Cone.msh"));
	

	
	
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glBindFramebuffer(GL_FRAMEBUFFER, 0);


	//sceneNode
	


	sceneTransforms.resize(5);
	treeTransforms.resize(4);


	sceneTime = 0.0f;
	waterRotate = 0.0f;
	waterCycle = 0.0f;

	currentFrame = 0;
	frameTime = 0.0f;
	soldierX = 5.0;
	soldierZ =5.0;


	init = true;
}
Renderer::~Renderer(void)
{
	glDeleteTextures(1, &shadowTex);
	glDeleteFramebuffers(1, &shadowFBO);
	glDeleteTextures(1, &earthTex);
	glDeleteTextures(1, &texture);

	for (auto& i : sceneMeshes)
		delete i;
	
	delete camera;
	delete quad;
	delete cube;
	delete sceneShader;
	delete shadowShader;
	delete treeShader;
	delete heightMap;
	delete reflectShader;
	delete skyboxShader;
	delete lightShader;
	delete basicShader;
	delete soldier;
	delete soldierShader;
	delete pointLightShader;
	delete combineShader;
	delete anim;
	delete material;
	delete root;
	
	
	

	
	


	delete light;
	delete mapLight;
}

void Renderer::UpdateScene(float dt)
{


	camera->UpdateCamera(dt);
	viewMatrix = camera->BuildViewMatrix();
	sceneTime += dt;
	waterRotate += dt * 2.0f; // 2 degrees a second
	waterCycle += dt * 0.25f; // 10 units a second
	projMatrix = Matrix4::Perspective(1.0f, 20000.0f, (float)width / (float)height, 45.0f);
	light->SetPosition(Vector3(sin(sceneTime * 0.8) * 8 + 4, sin(sceneTime * 0.8) * 5, cos(sceneTime * 0.8) * 8 + 4)*2000);
	frameFrustum.FromMatrix(projMatrix * viewMatrix);
	root->Update(dt);
	
	

	
	
	Vector3 heightMapSize = heightMap->GetHeightMapSize();
	/*for (int i = 1; i < 4; ++i)
	{
		Vector3 t = Vector3(-10 + (5 * i), 2.0f + sin(sceneTime * i), 0);
		sceneTransforms[i] = Matrix4::Translation(t) * Matrix4::Rotation(sceneTime * 10 * i, Vector3(1, 0, 0));
	}*/

	
	//soldier
	frameTime -= dt;
	while (frameTime < 0.0f)
	{
		currentFrame = (currentFrame + 1) % anim->GetFrameCount();
		frameTime += 1.0f / anim->GetFrameRate();
	}

	//tree
	int treeScale = 500;
	int rootScale = 500;
	int treeX = 10;
	int treeZ = 10;
	int treeY = heightMap->GetHeightAtCoord(treeX * treeScale, treeZ * treeScale);
	

	treeTransforms[0] = Matrix4::Translation(Vector3(treeX*500,treeY+500, treeZ*500)) * Matrix4::Scale(Vector3(200,500,200)) * Matrix4::Rotation(-sin(sceneTime) * 5, Vector3(1, 0, 0));
	treeTransforms[1] = Matrix4::Translation(Vector3(treeX*500+sin(sceneTime)*100, treeY + 1500, treeZ * 500)) * Matrix4::Scale(Vector3(500, 500, 500)) * Matrix4::Rotation(-90 - sin(sceneTime) * 5, Vector3(1, 0, 0));
	treeTransforms[2] = Matrix4::Translation(Vector3(treeX * 500 + sin(sceneTime) * 150, treeY+2000, treeZ * 500) )* Matrix4::Scale(Vector3(500, 500, 500)) * Matrix4::Rotation(-90 - sin(sceneTime) * 10, Vector3(1, 0, 0));
	treeTransforms[3] = Matrix4::Translation(Vector3(treeX * 500 + sin(sceneTime) * 200, treeY+2500, treeZ * 500) )* Matrix4::Scale(Vector3(500, 500, 500)) * Matrix4::Rotation(-90 - sin(sceneTime) *15, Vector3(1, 0, 0));
	treeTransforms[3] = Matrix4::Translation(Vector3(treeX * 500 + sin(sceneTime) * 200, treeY+2500, treeZ * 500) )* Matrix4::Scale(Vector3(500, 500, 500)) * Matrix4::Rotation(-90 - sin(sceneTime) *15, Vector3(1, 0, 0));

	//mainScene


	int r = 10;
	int scale0 = 500;
	int scale1 = 2000;
	int scale2 = 500;
	int scale3 = 1000;
	

	mapLight->SetPosition(heightMapSize *sin(sceneTime*0.8));
	
	mapLight->SetColour(Vector4(sin(sceneTime*0.8)+0.8, sin(sceneTime * 0.8)+0.8, sin(sceneTime * 0.8)+0.8, 1));
	
	sceneTransforms[0] = Matrix4::Translation(Vector3(17.5, 5+sin(sceneTime * 3), 2)* scale0) * Matrix4::Scale(Vector3(scale0, scale0, scale0)) * Matrix4::Rotation(1, Vector3(1, 0, 0));
	sceneTransforms[1] = Matrix4::Translation(Vector3(sin(sceneTime*0.8)*8+4,  sin(sceneTime*0.8)*5, cos(sceneTime*0.8) * 8 + 4) * scale1) * Matrix4::Scale(Vector3(scale1, scale1, scale1)) * Matrix4::Rotation(sceneTime * 10, Vector3(1, 0, 0));
	sceneTransforms[2] = Matrix4::Translation(Vector3(r * cos(sceneTime * 0.5)+4, 16, r * sin(sceneTime * 0.5)+4)* scale2)* Matrix4::Scale(Vector3(scale2, scale2, scale2)) * Matrix4::Rotation(sceneTime * 20 * 1, Vector3(1, 0, 0));
	sceneTransforms[3]=  Matrix4::Translation(Vector3(heightMapSize.x*0.5, 2*scale3, heightMapSize.z * 0.5)) * Matrix4::Scale(Vector3(scale3, scale3, scale3)) * Matrix4::Rotation(sceneTime * 10, Vector3(sin(sceneTime), 1, 1));
	sceneTransforms[4] = Matrix4::Translation(Vector3(1,4,1)*scale1) * Matrix4::Scale(Vector3(scale1, scale1, scale1)) * Matrix4::Rotation(sceneTime * 10, Vector3(1, 0, 0));

	if (Window::GetKeyboard()->KeyDown(KEYBOARD_3)) {
		isFollowingEnabled = false;
		camera->SetPosition(Vector3(-10, 18, -10) * scale2);
		camera->SetPitch(-20.0);
		camera->SetYaw(220.0);

	}

	
}
void Renderer::RenderScene()
{
	
	
	glViewport(0,0, width, height);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	
	BuildNodeLists(root);
	SortNodeLists();
	
	DrawSkybox();
	DrawSoldier(sceneTime);
	DrawHeightMap();
	DrawTree(sceneTime);
	DrawWater();
	DrawMainScene(sceneTime);
	DrawNodes();
	/*DrawShadowScene();*/
	


	
	
	glViewport(0, 0, width / 6, height / 6);
	DrawSkybox();
	DrawNodes();
	DrawShadowScene();
	ClearNodeLists();

}

void Renderer::BuildNodeLists(SceneNode* from)
{


	
	if (frameFrustum.InsideFrustum(*from))
	{
		
		Vector3 dir = from->GetWorldTransform().GetPositionVector() - camera->GetPosition();
		from->SetCameraDistance(Vector3::Dot(dir, dir));

		if (from->GetColour().w < 1.0f)
		{
			transparentNodeList.push_back(from);
		}
		else
		{
			nodeList.push_back(from);
		}
	}

	for (vector<SceneNode*>::const_iterator i = from->GetChildIteratorStart(); i != from->GetChildIteratorEnd(); ++i)
	{
		BuildNodeLists((*i));
	}
}


void Renderer::SortNodeLists()
{
	std::sort(transparentNodeList.rbegin(), transparentNodeList.rend(), SceneNode::CompareByCameraDistance);
	std::sort(nodeList.begin(), nodeList.end(), SceneNode::CompareByCameraDistance);
}

void Renderer::ClearNodeLists()
{
	transparentNodeList.clear();
	nodeList.clear();
}

void Renderer::DrawNodes()
{
	for (const auto& i : nodeList)
	{
		DrawNode(i);
	}
	for (const auto& i : transparentNodeList)
	{
		DrawNode(i);
	}
}


void Renderer::DrawNode(SceneNode* n)
{
	BindShader(shader);
	UpdateShaderMatrices();
	
	glUniform1i(glGetUniformLocation(shader->GetProgram(), "diffuseTex"), 0);
	
	if (n->GetMesh())
	{
		
		Matrix4 model = n->GetWorldTransform() * Matrix4::Scale(n->GetModelScale()*sin(sceneTime));
		glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "modelMatrix"), 1, false, model.values);
		glUniform4fv(glGetUniformLocation(shader->GetProgram(), "nodeColour"), 1, (float*)&n->GetColour());
		texture = n->GetTexture();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);

		glUniform1i(glGetUniformLocation(shader->GetProgram(), "useTexture"), texture);
		
		n->Draw(*this);
	}

	for (vector<SceneNode*>::const_iterator i = n->GetChildIteratorStart(); i != n->GetChildIteratorEnd(); ++i)
	{
		
		DrawNode(*i);
	}
}

void Renderer::DrawTree(float dt) {
	
	sceneTime = dt;
	BindShader(lightShader);
	SetShaderLight(*mapLight);


	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "shadowTex"), 2);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, treeRoot);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, sceneBump);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	
	modelMatrix = treeTransforms[0];
	UpdateShaderMatrices();
	treeMeshs[0]->Draw();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, tree);

	modelMatrix = treeTransforms[1];
	UpdateShaderMatrices();
	treeMeshs[1]->Draw();

	modelMatrix = treeTransforms[2];
	UpdateShaderMatrices();
	treeMeshs[2]->Draw();

	modelMatrix = treeTransforms[3];
	UpdateShaderMatrices();
	treeMeshs[3]->Draw();
}


void Renderer::DrawSkybox()
{
	glDepthMask(GL_FALSE);

	BindShader(skyboxShader);

	UpdateShaderMatrices();

	quad->Draw();

	glDepthMask(GL_TRUE);
}

void Renderer::DrawHeightMap()
{
	
	BindShader(lightShader);
	SetShaderLight(*mapLight);
	glUniform3fv(glGetUniformLocation(lightShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "diffuseTex"), 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earthTex);

	glUniform1i(glGetUniformLocation(lightShader->GetProgram(), "bumpTex"), 1);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, earthBump);

	

	modelMatrix.ToIdentity();
	textureMatrix.ToIdentity();

	UpdateShaderMatrices();

	heightMap->Draw();
}
void Renderer::DrawWater()
{
	BindShader(reflectShader);

	glUniform3fv(glGetUniformLocation(reflectShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(reflectShader->GetProgram(), "cubeTex"), 2);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, waterTex);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMap);

	

	Vector3 hSize = heightMap->GetHeightMapSize();

	modelMatrix = Matrix4::Translation(hSize * Vector3(0.5f,0.2f+sin(sceneTime*0.2)*0.1, 0.5f)) * Matrix4::Scale(hSize * 0.5f) * Matrix4::Rotation(90, Vector3(1, 0, 0));
	textureMatrix = Matrix4::Translation(Vector3(waterCycle, 0.0f, waterCycle)) * Matrix4::Scale(Vector3(10, 10, 10)) * Matrix4::Rotation(waterRotate, Vector3(0, 0, 1));

	UpdateShaderMatrices();
	quad->Draw();
}


void Renderer::DrawSoldier(float dt)
{
	

	BindShader(soldierShader);
	glUniform1i(glGetUniformLocation(soldierShader->GetProgram(), "diffuseTex"), 0);
	
	Vector3 hSize = heightMap->GetHeightMapSize();
	int soldierScale = 500;
	
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_UP)){
		soldierZ = soldierZ + 0.1;
	}
		
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_DOWN))
		soldierZ = soldierZ-0.1;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_LEFT))
		soldierX = soldierX +0.1;
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_RIGHT))
		soldierX = soldierX -0.1;
	
	
	
	
	int soldierY = heightMap->GetHeightAtCoord(soldierX* soldierScale, soldierZ* soldierScale);
	
	
	modelMatrix = Matrix4::Translation(Vector3(soldierX* soldierScale, soldierY, soldierZ* soldierScale)) * Matrix4::Scale(Vector3(soldierScale, soldierScale, soldierScale)) * Matrix4::Rotation(1, Vector3(1, 0, 0));
	vector<Matrix4> frameMatrices;
	//follow 
	if (Window::GetKeyboard()->KeyDown(KEYBOARD_2)) {

		isFollowingEnabled = !isFollowingEnabled;
		isTrackEnabled = false;
	}

	if(isFollowingEnabled){
		camera->SetPosition(Vector3(soldierX * soldierScale, soldierY + 1500, soldierZ * soldierScale - 2500));
		camera->SetPitch(-15.0);
		camera->SetYaw(180.0);
	}

	

	const Matrix4* invBindPose = soldier->GetInverseBindPose();
	const Matrix4* frameData = anim->GetJointData(currentFrame);

	UpdateShaderMatrices();

	for (unsigned int i = 0; i < soldier->GetJointCount(); ++i)
	{
		frameMatrices.emplace_back(frameData[i] * invBindPose[i]);
	}

	int j = glGetUniformLocation(soldierShader->GetProgram(), "joints");
	glUniformMatrix4fv(j, frameMatrices.size(), false, (float*)frameMatrices.data());

	for (int i = 0; i < soldier->GetSubMeshCount(); ++i)
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, matTextures[i]);
		soldier->DrawSubMesh(i);
	}
}
void Renderer::DrawShadowScene()
{
	

	BindShader(shadowShader);

	viewMatrix = Matrix4::BuildViewMatrix(light->GetPosition(), Vector3(0, 0, 0));
	projMatrix = Matrix4::Perspective(1, 100, 1, 45);
	shadowMatrix = projMatrix * viewMatrix;

	for (int i = 0; i < 3; ++i)
	{
		modelMatrix = sceneTransforms[i];
		UpdateShaderMatrices();
		sceneMeshes[i]->Draw();
	}

	glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
	glViewport(0, 0, width, height);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
void Renderer::DrawMainScene(float dt)
{
	sceneTime = dt;
	BindShader(sceneShader);
	SetShaderLight(*light);
	

	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "diffuseTex"), 0);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "bumpTex"), 1);
	glUniform1i(glGetUniformLocation(sceneShader->GetProgram(), "shadowTex"), 2);
	glUniform3fv(glGetUniformLocation(sceneShader->GetProgram(), "cameraPos"), 1, (float*)&camera->GetPosition());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sceneDiffuse);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, sceneBump);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, shadowTex);

	//for (int i = 0; i < 3; ++i)
	//{
	//	modelMatrix = sceneTransforms[i];
	//	UpdateShaderMatrices();
	//	sceneMeshes[i]->Draw();
	//}

	modelMatrix = sceneTransforms[0];
	UpdateShaderMatrices();
	sceneMeshes[0]->Draw();

	modelMatrix = sceneTransforms[2];
	UpdateShaderMatrices();
	sceneMeshes[2]->Draw();

	BindShader(basicShader);
	SetShaderLight(*light);
	glUniform1i(glGetUniformLocation(basicShader->GetProgram(), "diffuseTex"), 0);
	UpdateShaderMatrices();

	if (std::fmod(sceneTime, 0.4) < 0.8) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sun1);

	}

	if (std::fmod(sceneTime, 0.4) < 0.6) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sun2);
	}

	if (std::fmod(sceneTime, 0.4) < 0.4) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sun3);
	}
	if (std::fmod(sceneTime, 0.4) < 0.2) {
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, sun4);
	}




	modelMatrix = sceneTransforms[1];
	UpdateShaderMatrices();
	sceneMeshes[1]->Draw();

	

	BindShader(basicShader);
	glUniform1i(glGetUniformLocation(basicShader->GetProgram(), "diffuseTex"), 0);
	

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, glass);


	
	modelMatrix = sceneTransforms[3];
	UpdateShaderMatrices();
	sceneMeshes[3]->Draw();

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, earth);

	modelMatrix = sceneTransforms[4];
	UpdateShaderMatrices();
	sceneMeshes[4]->Draw();



	



}