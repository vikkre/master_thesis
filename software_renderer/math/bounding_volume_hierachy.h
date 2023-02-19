#pragma once

#include <vector>
#include <list>
#include <functional>

#include "ray.h"
#include "aabb.h"


class BVH {
	public:
		struct Data {
			AABB aabb;
			size_t elemIndex;
		};

		struct Node {
			AABB aabb;
			Node* left;
			Node* right;
			size_t elemIndex;
		};

		BVH();
		BVH(const BVH& other);
		BVH& operator=(const BVH& other);
		~BVH();

		void init(const std::vector<Data>& inputs);
		void rebuild(std::function<AABB(size_t)> getAABB);
		std::vector<size_t> getHits(const Ray& ray) const;
	
	private:
		std::vector<size_t> getIntersections(const Node* node, const Ray& ray) const;
		Node* copyNode(Node* oldNode);
		void deleteNodes(Node* node);
		void rebuildNode(Node* node, std::function<AABB(size_t)> getAABB);

		Node* root;
};
