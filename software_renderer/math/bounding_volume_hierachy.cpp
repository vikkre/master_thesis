#include "bounding_volume_hierachy.h"


struct ClosestPair {
	std::list<BVH::Node*>::iterator left, right;
	float distance2;
};

ClosestPair findClosestPair(std::list<BVH::Node*>& nodes) {
	ClosestPair cp;
	cp.distance2 = INFINITY;

	for (std::list<BVH::Node*>::iterator i = nodes.begin(); i != nodes.end(); ++i) {
		for (std::list<BVH::Node*>::iterator j = i; j != nodes.end(); ++j) {
			if (i == j) continue;

			float d2 = (*i)->aabb.getCenter().distanceSquared((*j)->aabb.getCenter());
			if (d2 < cp.distance2) {
				cp.left = i;
				cp.right = j;
				cp.distance2 = d2;
			}
		}
	}

	return cp;
}


BVH::BVH()
:root(nullptr) {}

BVH::BVH(const BVH& other) {
	root = copyNode(other.root);
}

BVH& BVH::operator=(const BVH& other) {
	if (root != nullptr) deleteNodes(root);
	root = copyNode(other.root);
	return *this;
}

BVH::~BVH() {
	if (root != nullptr) deleteNodes(root);
}

void BVH::init(const std::vector<Data>& inputs) {
	std::list<Node*> nodes;
	for (const Data& data: inputs) {
		Node* node = new Node();
		node->left  = nullptr;
		node->right = nullptr;
		node->aabb = data.aabb;
		node->elemIndex = data.elemIndex;
		nodes.push_back(node);
	}

	while (nodes.size() > 1) {
		Node* node = new Node();

		ClosestPair cp = findClosestPair(nodes);

		std::list<Node*>::iterator left = cp.left;
		node->left = *left;
		nodes.erase(left);

		std::list<Node*>::iterator right = cp.right;
		node->right = *right;
		nodes.erase(right);

		node->aabb = AABB(node->left->aabb, node->right->aabb);
		nodes.push_back(node);
	}
	root = nodes.front();
}

void BVH::rebuild(std::function<AABB(size_t)> getAABB) {
	rebuildNode(root, getAABB);
}

std::vector<size_t> BVH::getHits(const Ray& ray) const {
	return getIntersections(root, ray);
}

std::vector<size_t> BVH::getIntersections(const Node* node, const Ray& ray) const {
	bool hit = node->aabb.doesRayIntersect(ray);
	if (!hit) return {};

	bool leaf = node->left == nullptr;
	if (leaf) return { node->elemIndex };

	std::vector<size_t> ret;
	
	std::vector<size_t> leftRes  = getIntersections(node->left,  ray);
	std::vector<size_t> rightRes = getIntersections(node->right, ray);

	ret.insert(ret.end(), leftRes.begin(),  leftRes.end() );
	ret.insert(ret.end(), rightRes.begin(), rightRes.end());

	return ret;
}

BVH::Node* BVH::copyNode(Node* oldNode) {
	Node* newNode = new Node();

	bool leaf = oldNode->left == nullptr;
	if (leaf) {
		newNode->aabb = oldNode->aabb;
		newNode->elemIndex = oldNode->elemIndex;
		newNode->left  = nullptr;
		newNode->right = nullptr;
	} else {
		newNode->aabb = oldNode->aabb;
		newNode->left  = copyNode(oldNode->left );
		newNode->right = copyNode(oldNode->right);
	}

	return newNode;
}

void BVH::deleteNodes(Node* node) {
	bool leaf = node->left == nullptr;
	if (!leaf) {
		deleteNodes(node->left );
		deleteNodes(node->right);
	}
	delete node;
}

void BVH::rebuildNode(Node* node, std::function<AABB(size_t)> getAABB) {
	bool leaf = node->left == nullptr;
	if (leaf) {
		node->aabb = getAABB(node->elemIndex);
	} else {
		rebuildNode(node->left,  getAABB);
		rebuildNode(node->right, getAABB);
		node->aabb = AABB(node->left->aabb, node->right->aabb);
	}
}
