#pragma once
#include "../nclgl/SceneNode.h"

class Tree :public SceneNode {
public:
	Tree(Mesh* cone,Mesh* cylinder);
	~Tree(void) {};
	void Update(float dt) override;


protected:
	SceneNode* root;
	SceneNode* tree1;
	SceneNode* tree2;
	SceneNode* tree3;
};