#include "stdafx.h"
#include "KDNode.h"
#include <algorithm>
#include "ObjectManager.h"
#include "BufferMananger.h"

#include "Global.h"

using namespace Global;

KDNode::KDNode()
	:m_left(nullptr), m_right(nullptr), m_bbox({ zero3, zero3 }), m_triangles({}), m_spheres({}), m_axis(0)
{

}

KDNode::KDNode(BoundingBox bbox, 
	KDNode* left, KDNode* right, 
	std::vector<Triangle> triangles, std::vector<Sphere> spheres, char axis)
	: m_bbox(bbox), m_left(left), m_right(right), m_triangles(triangles), m_spheres(spheres), m_axis(axis)
{
}

KDNode::KDNode(const KDNode& other)
	: m_bbox(other.m_bbox), m_triangles(other.m_triangles), m_spheres(other.m_spheres), m_axis(other.m_axis)
{
	if (m_left)
		delete m_left;
	m_left = nullptr;
	m_left = new KDNode(*other.m_left);

	if (m_right)
		delete m_right;
	m_left = nullptr;
	m_right = new KDNode(*other.m_right);
}

KDNode& KDNode::operator=(const KDNode& other)
{
	m_bbox = other.m_bbox;
	m_triangles = other.m_triangles;
	m_spheres = other.m_spheres;
	m_axis = other.m_axis;

	if (m_left)
		delete m_left;
	m_left = nullptr;
	m_left = new KDNode(*other.m_left);

	if (m_right)
		delete m_right;
	m_left = nullptr;
	m_right = new KDNode(*other.m_right);
	
	return *this;
}

bool KDNode::operator==(const KDNode& other){
	if (m_bbox != other.m_bbox)
		return false;
	if (m_triangles != other.m_triangles)
		return false;
	if (m_spheres != other.m_spheres)
		return false;
	if (m_left != other.m_left)
		return false;
	if (m_right != other.m_right)
		return false;
	if (m_axis != other.m_axis)
		return false;
	return true;
}

KDNode::~KDNode()
{
	if (m_left)
		delete m_left;
	m_left = nullptr;

	if (m_right)
		delete m_right;
	m_right = nullptr;
}



void KDTree::build(KDNode* node, int depth)
{
	enum SplittingAxis{
		WIDTH, //x-axis
		HEIGHT, //y-axis
		DEPTH //z-axis
	};

	auto median = [](std::vector<float> vec){
		std::sort(vec.begin(), vec.end());
		int size = (int)vec.size();
		float med = 0.0f;
		if (size % 2 == 0){
			med = (vec[(size / 2) - 1] + vec[size / 2]) / 2.0f;
		}
		else{
			med = vec[size / 2];
		}
		return med;
	};

	auto axis = [](std::vector<cl_float3> vec){
		cl_float3 minvec = zero3;
		cl_float3 maxvec = zero3;
		for (int i = 0; i < vec.size(); ++i){
			cl_float3 curvec = vec[i];

			if (curvec.x < minvec.x)
				minvec.x = curvec.x;
			else if (curvec.x > maxvec.x)
				maxvec.x = curvec.x;

			if (curvec.y < minvec.y)
				minvec.y = curvec.y;
			else if (curvec.y > maxvec.y)
				maxvec.y = curvec.y;

			if (curvec.z < minvec.z)
				minvec.z = curvec.z;
			else if (curvec.z > maxvec.z)
				maxvec.z = curvec.z;
		}
		
		float x = maxvec.x - minvec.x, y = maxvec.y - minvec.y, z = maxvec.z - minvec.z;
		if (x > y)
			if (x > z)
				return WIDTH;
			else
				return DEPTH;
		else
			if (y > z)
				return HEIGHT;
			else
				return DEPTH;
	};

	if (depth < 100 && (node->m_triangles.size() + node->m_spheres.size() > 1))
	{
		std::vector<float> x, y, z;
		std::vector<cl_float3> position;
		for (int t = 0; t < node->m_triangles.size(); ++t)
		{
			x.push_back(node->m_triangles[t].boundingBox().position.x);
			y.push_back(node->m_triangles[t].boundingBox().position.y);
			y.push_back(node->m_triangles[t].boundingBox().position.y);
			position.push_back(node->m_triangles[t].boundingBox().position);
		}
		for (int s = 0; s < node->m_spheres.size(); ++s)
		{
			x.push_back(node->m_spheres[s].boundingBox().position.x);
			y.push_back(node->m_spheres[s].boundingBox().position.y);
			z.push_back(node->m_spheres[s].boundingBox().position.z);
			position.push_back(node->m_spheres[s].boundingBox().position);
		}

		SplittingAxis splitAxis = axis(position);
		node->m_axis = static_cast<char>(splitAxis);

		cl_float3 leftcenter = node->m_bbox.position;
		cl_float3 rightcenter = node->m_bbox.position;
		cl_float3 dimensions = node->m_bbox.dimensions;

		float medianf = 0.0f;
		switch (splitAxis){
			case WIDTH: {
				leftcenter.x -= node->m_bbox.dimensions.x / 2.0f;
				rightcenter.x += node->m_bbox.dimensions.x / 2.0f;
				dimensions.x /= 2.0f;
				medianf = median(x);
				break;
			}
			case HEIGHT: {
				leftcenter.y -= node->m_bbox.dimensions.y / 2.0f;
				rightcenter.y += node->m_bbox.dimensions.y / 2.0f;
				dimensions.y /= 2.0f;
				medianf = median(y);
				break;
			}
			case DEPTH: {
				leftcenter.z -= node->m_bbox.dimensions.z / 2.0f;
				rightcenter.z += node->m_bbox.dimensions.z / 2.0f;
				dimensions.z /= 2.0f;
				medianf = median(z);
				break;
			}
		}

		std::vector<Triangle> lefttriangles, righttriangles;
		std::vector<Sphere> leftspheres, rightspheres;

		for (int t = 0; t < node->m_triangles.size(); ++t){
			switch (splitAxis){
				case WIDTH: {
					if (node->m_triangles[t].boundingBox().position.x < medianf)
						lefttriangles.push_back(node->m_triangles[t]);
					else
						righttriangles.push_back(node->m_triangles[t]);
					break;
				}
				case HEIGHT: {
					if (node->m_triangles[t].boundingBox().position.y < medianf)
						lefttriangles.push_back(node->m_triangles[t]);
					else
						righttriangles.push_back(node->m_triangles[t]);
					break;
				}
				case DEPTH: {
					if (node->m_triangles[t].boundingBox().position.z < medianf)
						lefttriangles.push_back(node->m_triangles[t]);
					else
						righttriangles.push_back(node->m_triangles[t]);
					break;
				}
			}
		}

		for (int s = 0; s < node->m_spheres.size(); ++s){
			switch (splitAxis){
				case WIDTH: {
					if (node->m_spheres[s].boundingBox().position.x < medianf)
						leftspheres.push_back(node->m_spheres[s]);
					else
						rightspheres.push_back(node->m_spheres[s]);
					break;
				}
				case HEIGHT: {
					if (node->m_spheres[s].boundingBox().position.y < medianf)
						leftspheres.push_back(node->m_spheres[s]);
					else
						rightspheres.push_back(node->m_spheres[s]);
					break;
				}
				case DEPTH: {
					if (node->m_spheres[s].boundingBox().position.z < medianf)
						leftspheres.push_back(node->m_spheres[s]);
					else
						rightspheres.push_back(node->m_spheres[s]);
					break;
				}
			}

			node->m_left = new KDNode(BoundingBox{ leftcenter, dimensions }, nullptr, nullptr, lefttriangles, leftspheres);
			node->m_right = new KDNode(BoundingBox{ rightcenter, dimensions }, nullptr, nullptr, righttriangles, rightspheres);

			build(node->m_left, depth + 1);
			build(node->m_right, depth + 1);
		}
	}
}

void KDNode::nodes(std::vector<KDNode*> &nodes)
{
	nodes.push_back(this);
	if (m_left)
		m_left->nodes(nodes);
	if (m_right)
		m_right->nodes(nodes);
}

std::vector<KDNode*> KDTree::nodes()
{
	std::vector<KDNode*> nodes;
	m_root->nodes(nodes);
	return nodes;
}


int KDTree::nodeCount(){
	return this->nodes().size();
}

KDTree::KDTree(BoundingBox bbox, ObjectManager & objMgr, BufferManager & buffMgr)
	: m_bbox(bbox), m_objMgr(objMgr), m_buffMgr(buffMgr), m_triangles(objMgr.triangles()), m_spheres(objMgr.spheres())
{
	m_root = new KDNode(bbox, nullptr, nullptr, m_triangles, m_spheres);
	build(m_root);
}

void KDTree::createKDTreeBuffers()
{
	std::vector<KDNode*> nodes;
	m_root->nodes(nodes);

	int nodeCount = nodes.size();
	int triCount = m_triangles.size();
	int sprCount = m_spheres.size();

	m_positions = (cl_float3*)malloc(sizeof(cl_float3) * nodeCount);
	m_dimensions = (cl_float3*)malloc(sizeof(cl_float3) * nodeCount);

	m_axis = (cl_char*)malloc(sizeof(cl_char) * nodeCount);

	m_triCount = (cl_int*)malloc(sizeof(cl_int) * nodeCount);
	m_sprCount = (cl_int*)malloc(sizeof(cl_int) * nodeCount);

	m_left = (cl_int*)malloc(sizeof(cl_int) * nodeCount);
	m_right = (cl_int*)malloc(sizeof(cl_int) * nodeCount);

	m_triOffset = (cl_int*)malloc(sizeof(cl_int) * nodeCount);
	m_sprOffset = (cl_int*)malloc(sizeof(cl_int) * nodeCount);

	auto nodeIndex = [](KDNode* node, std::vector<KDNode*> nodeList){
		for (int n = 0; n < nodeList.size(); ++n){
			if (node == nodeList[n])
				return n;
		}
		return -1;
	};

	int triIdxCnt = 0, sprIdxCnt = 0;
	for (int i = 0; i < nodeCount; ++i)
	{
		m_positions[i] = nodes[i]->m_bbox.position;
		m_dimensions[i] = nodes[i]->m_bbox.dimensions;
		m_axis[i] = nodes[i]->m_axis;
		m_triCount[i] = nodes[i]->m_triangles.size();
		m_sprCount[i] = nodes[i]->m_spheres.size();
		m_left[i] = nodeIndex(nodes[i]->m_left, nodes);
		m_right[i] = nodeIndex(nodes[i]->m_right, nodes);
		triIdxCnt += m_triCount[i];
		sprIdxCnt += m_sprCount[i];

		if (i > 0){
			m_triOffset[i] = m_triCount[i - 1];
			m_sprOffset[i] = m_sprCount[i - 1];
		}
		else{
			m_triOffset[i] = 0;
			m_sprOffset[i] = 0;
		}
	}

	m_triindices = (cl_int*)malloc(sizeof(cl_int) * triIdxCnt);
	m_sprindices = (cl_int*)malloc(sizeof(cl_int) * sprIdxCnt);

	for (int i = 0; i < nodeCount; ++i)
	{
		for (int t = 0; t < m_triCount[i]; ++t)
		{
			m_triindices[m_triOffset[i] + t] = m_objMgr.triangleIndex(nodes[i]->m_triangles[t]);
		}
		for (int s = 0; s < m_sprCount[i]; ++s)
		{
			m_sprindices[m_sprOffset[i] + s] = m_objMgr.sphereIndex(nodes[i]->m_spheres[s]);
		}
	}

	m_buffMgr.createBuffer("positions", sizeof(cl_float3) * nodeCount, m_positions);
	m_buffMgr.createBuffer("dimensions", sizeof(cl_float3) * nodeCount, m_dimensions);
	m_buffMgr.createBuffer("axis", sizeof(cl_char) * nodeCount, m_axis);
	m_buffMgr.createBuffer("kdLeft", sizeof(cl_int) * nodeCount, m_left);
	m_buffMgr.createBuffer("kdRight", sizeof(cl_int) * nodeCount, m_right);

	m_buffMgr.createBuffer("kdTriIndices", sizeof(cl_int) * triIdxCnt, m_triindices);
	m_buffMgr.createBuffer("kdTriCount", sizeof(cl_int) * nodeCount, m_triCount);
	m_buffMgr.createBuffer("kdTriOffset", sizeof(cl_int) * nodeCount, m_triOffset);

	m_buffMgr.createBuffer("kdSprIndicies", sizeof(cl_int) * sprIdxCnt, m_sprindices);
	m_buffMgr.createBuffer("kdSprCount", sizeof(cl_int) * nodeCount, m_sprCount);
	m_buffMgr.createBuffer("kdSprOffset", sizeof(cl_int) * nodeCount, m_sprOffset);

}

KDTree::KDTree(const KDTree& other)
	: m_triangles(other.m_triangles), m_spheres(other.m_spheres), m_bbox(other.m_bbox), m_objMgr(other.m_objMgr), m_buffMgr(other.m_buffMgr)
{
	if (m_root)
		delete m_root;
	m_root = nullptr;
	m_root = new KDNode(*other.m_root);
}

KDTree& KDTree::operator=(const KDTree& other)
{
	m_objMgr = other.m_objMgr;
	m_buffMgr = other.m_buffMgr;
	m_triangles = other.m_triangles;
	m_spheres = other.m_spheres;
	m_bbox = other.m_bbox;

	if (m_root)
		delete m_root;
	m_root = nullptr;
	m_root = new KDNode(*other.m_root);

	return *this;
}

KDTree::~KDTree()
{
	if (m_root)
		delete m_root;
	m_root = nullptr;

	free(m_positions);
	free(m_dimensions);
	free(m_axis);
	free(m_left);
	free(m_right);
	free(m_triindices);
	free(m_triCount);
	free(m_triOffset);
	free(m_sprindices);
	free(m_sprCount);
	free(m_sprOffset);
}