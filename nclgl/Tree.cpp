#include "Tree.h"

Tree::Tree(Mesh* cone, Mesh* cylinder) {
	

	SceneNode* root = new SceneNode(cylinder);
	root->SetModelScale(Vector3(5, 10, 5));
	root->SetTransform(Matrix4::Translation(Vector3(5, 5, 5)));
	root-> SetBoundingRadius(15.0f);
	AddChild(root);

	tree1 = new SceneNode(cone);
	tree1->SetModelScale(Vector3(10, 10, 10));
	tree1->SetTransform(Matrix4::Translation(Vector3(5, 6, 5)));
	tree1->SetBoundingRadius(15.0f);
	tree1->AddChild(root);

	tree2 = new SceneNode(cone);
	tree2->SetModelScale(Vector3(7, 7, 7));
	tree2->SetTransform(Matrix4::Translation(Vector3(5, 7, 5)));
	tree2->SetBoundingRadius(15.0f);
	tree2->AddChild(root);

	tree3 = new SceneNode(cone);
	tree3->SetModelScale(Vector3(5, 5, 5));
	tree3->SetTransform(Matrix4::Translation(Vector3(5, 8, 5)));
	tree3->SetBoundingRadius(15.0f);
	tree3->AddChild(root);

}

void Tree::Update(float dt) {
	transform = transform * Matrix4::Rotation(30.0f * dt, Vector3(0, 1, 0));

	SceneNode::Update(dt);
}