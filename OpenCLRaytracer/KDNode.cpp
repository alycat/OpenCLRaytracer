#include "stdafx.h"
#include "KDNode.h"
#include <algorithm>
#include "ObjectManager.h"
#include "BufferMananger.h"
#include "Global.h"

using namespace Global;

bool sortX(cl_float3 &lhs, cl_float3 &rhs){
	return lhs.x < rhs.x;
}

bool sortY(cl_float3 &lhs, cl_float3 &rhs){
	return lhs.y < rhs.y;
}

bool sortZ(cl_float3 &lhs, cl_float3 &rhs){
	return lhs.z < rhs.z;
}

KDNode::KDNode()
	:m_left(nullptr), m_right(nullptr), m_bbox({ zero3, zero3 }), m_triangles({}), m_spheres({}), m_positions({}), m_axis(0)
{

}

KDNode::KDNode(BoundingBox bbox, 
	KDNode* left, KDNode* right, 
	std::vector<Triangle> triangles, std::vector<Sphere> spheres, std::vector<cl_float3> positions, char axis)
	: m_bbox(bbox), m_left(left), m_right(right), m_triangles(triangles), m_spheres(spheres), m_positions(positions), m_axis(axis)
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

	static auto median = [](std::vector<cl_float3> &vec, int i){
		static bool (*vectorSort[3])(cl_float3&, cl_float3&) = {sortX, sortY, sortZ};
		std::sort(vec.begin(), vec.end(), vectorSort[i]);
		int size = (int)vec.size();
		float med = 0.0f;
		if (size & 1){
			switch (i){
			case 0: med = vec[size / 2].x; break;
			case 1: med = vec[size / 2].y; break;
			case 2: med = vec[size / 2].z; break;
			}
		}
		else{
			switch (i){
			case 0: med = (vec[(size / 2) - 1].x + vec[size / 2].x) / 2.0f; break;
			case 1: med = (vec[(size / 2) - 1].y + vec[size / 2].y) / 2.0f; break;
			case 2: med = (vec[(size / 2) - 1].z + vec[size / 2].z) / 2.0f; break;
			}
		}
		return med;
	};

	static auto axis = [](std::vector<cl_float3> vec){
		cl_float3 minvec = zero3;
		cl_float3 maxvec = zero3;
		for (int i = 0; i < vec.size(); ++i){
			cl_float3 curvec = vec[i];

			if (curvec.x < minvec.x){
				minvec.x = curvec.x;
			}
			else if (curvec.x > maxvec.x){
				maxvec.x = curvec.x;
			}

			if (curvec.y < minvec.y){
				minvec.y = curvec.y;
			}
			else if (curvec.y > maxvec.y){
				maxvec.y = curvec.y;
			}

			if (curvec.z < minvec.z){
				minvec.z = curvec.z;
			}
			else if (curvec.z > maxvec.z){
				maxvec.z = curvec.z;
			}
		}
		
		float x = maxvec.x - minvec.x;
		float y = maxvec.y - minvec.y;
		float z = maxvec.z - minvec.z;

		if (x > y){
			if (x > z){
				return WIDTH;
			}
			else{
				return DEPTH;
			}
		}
		else{
			if (y > z){
				return HEIGHT;
			}
			else{
				return DEPTH;
			}
		}
	};

	if (depth < 10 && (node->m_triangles.size() + node->m_spheres.size()) > 2)
	{
		std::vector<cl_float3> positions = node->m_positions;

		SplittingAxis splitAxis = axis(positions);
		node->m_axis = static_cast<char>(splitAxis);

		cl_float3 leftcenter = node->m_bbox.position;
		cl_float3 rightcenter = node->m_bbox.position;
		cl_float3 dimensions = node->m_bbox.dimensions;
		cl_float3 leftdimensions = node->m_bbox.dimensions;
		cl_float3 rightdimensions = node->m_bbox.dimensions;

		float medianf = 0.0f;
		switch (splitAxis){
			case WIDTH: {
				medianf = median(positions, 0);
				leftdimensions.x = medianf - (node->m_bbox.position.x - (node->m_bbox.dimensions.x/2.0f));
				rightdimensions.x = (node->m_bbox.position.x + (node->m_bbox.dimensions.x / 2.0f)) - medianf;
				leftcenter.x = (node->m_bbox.position.x - (node->m_bbox.dimensions.x/2.0f) + medianf)/2.0f;// (node->m_bbox.dimensions.x / 2.0f);
				rightcenter.x = (node->m_bbox.position.x + (node->m_bbox.dimensions.x / 2.0f) + medianf)/2.0f;
				break;
			}
			case HEIGHT: {
				medianf = median(positions, 1);
				leftdimensions.y = medianf - (node->m_bbox.position.y - (node->m_bbox.dimensions.y / 2.0f));
				rightdimensions.y = (node->m_bbox.position.y + (node->m_bbox.dimensions.y / 2.0f)) - medianf;
				leftcenter.y = (node->m_bbox.position.y - (node->m_bbox.dimensions.y / 2.0f) + medianf) / 2.0f;// (node->m_bbox.dimensions.x / 2.0f);
				rightcenter.y = (node->m_bbox.position.y + (node->m_bbox.dimensions.y / 2.0f) + medianf) / 2.0f;
				break;
			}
			case DEPTH: {
				medianf = median(positions, 2);
				leftdimensions.z = medianf - (node->m_bbox.position.z - (node->m_bbox.dimensions.z / 2.0f));
				rightdimensions.z = (node->m_bbox.position.z + (node->m_bbox.dimensions.z / 2.0f)) - medianf;
				leftcenter.z = (node->m_bbox.position.z - (node->m_bbox.dimensions.z / 2.0f) + medianf) / 2.0f;// (node->m_bbox.dimensions.x / 2.0f);
				rightcenter.z = (node->m_bbox.position.z + (node->m_bbox.dimensions.z / 2.0f) + medianf) / 2.0f;
				break;
			}
		}

		std::vector<Triangle> lefttriangles, righttriangles;
		std::vector<Sphere> leftspheres, rightspheres;
		std::vector<cl_float3> leftpositions, rightpositions;

		for (int t = 0; t < node->m_triangles.size(); ++t){
			Triangle triangle = m_triangles[t];
			switch (splitAxis){
				case WIDTH: {
					if (node->m_triangles[t].boundingBox().position.x < medianf){
						lefttriangles.push_back(node->m_triangles[t]);
						leftpositions.push_back(triangle.boundingBox().position);
					}
					else{
						righttriangles.push_back(node->m_triangles[t]);
						rightpositions.push_back(triangle.boundingBox().position);
					}
					break;
				}
				case HEIGHT: {
					if (node->m_triangles[t].boundingBox().position.y < medianf){
						lefttriangles.push_back(node->m_triangles[t]);
						leftpositions.push_back(triangle.boundingBox().position);
					}
					else{
						righttriangles.push_back(node->m_triangles[t]);
						rightpositions.push_back(triangle.boundingBox().position);
					}
					break;
				}
				case DEPTH: {
					if (node->m_triangles[t].boundingBox().position.z < medianf){
						lefttriangles.push_back(node->m_triangles[t]);
						leftpositions.push_back(triangle.boundingBox().position);
					}
					else{
						righttriangles.push_back(node->m_triangles[t]);
						rightpositions.push_back(triangle.boundingBox().position);
					}
					break;
				}
			}
		}

		for (int s = 0; s < node->m_spheres.size(); ++s){
			Sphere sphere = node->m_spheres[s];
			switch (splitAxis){
				case WIDTH: {
					if (node->m_spheres[s].boundingBox().position.x < medianf){
						leftspheres.push_back(node->m_spheres[s]);
						leftpositions.push_back(sphere.center);
					}
					else{
						rightspheres.push_back(node->m_spheres[s]);
						rightpositions.push_back(sphere.center);
					}
					break;
				}
				case HEIGHT: {
					if (node->m_spheres[s].boundingBox().position.y < medianf){
						leftspheres.push_back(node->m_spheres[s]);
						leftpositions.push_back(sphere.center);
					}
					else{
						rightspheres.push_back(node->m_spheres[s]);
						rightpositions.push_back(sphere.center);
					}
					break;
				}
				case DEPTH: {
					if (node->m_spheres[s].boundingBox().position.z < medianf){
						leftspheres.push_back(node->m_spheres[s]);
						leftpositions.push_back(sphere.center);
					}
					else{
						rightspheres.push_back(node->m_spheres[s]);
						rightpositions.push_back(sphere.center);
					}
					break;
				}
			}

		}
		node->m_left = new KDNode(BoundingBox{ leftcenter, leftdimensions }, nullptr, nullptr, lefttriangles, leftspheres, leftpositions);
		node->m_right = new KDNode(BoundingBox{ rightcenter, rightdimensions }, nullptr, nullptr, righttriangles, rightspheres, rightpositions);

		build(node->m_left, depth + 1);
		build(node->m_right, depth + 1);
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
	return this->nodes().size();//max(this->nodes().size(), 30);
}

KDTree::KDTree(BoundingBox bbox, ObjectManager & objMgr, BufferManager & buffMgr)
	: m_bbox(bbox), m_objMgr(objMgr), m_buffMgr(buffMgr), m_triangles(objMgr.triangles()), m_spheres(objMgr.spheres())
{
	m_root = new KDNode(bbox, nullptr, nullptr, m_triangles, m_spheres, m_objMgr.positions());
	build(m_root);
	OutputDebugString("kd-tree built.\n");
}

struct TriangleIndexGenerator{
	cl_int* triCount;
	std::vector<KDNode*> nodes;
	ObjectManager &objMgr;

	TriangleIndexGenerator(cl_int* tc, std::vector<KDNode*> n, ObjectManager &om)
		:triCount(tc), nodes(n), objMgr(om){}

	cl_int operator()()
	{
		static int i = 0, t = 0;
		Triangle triangle = nodes[i]->m_triangles[t];

		++t;
		while (t >= triCount[i]){
			++i;
			t = 0;
		}
		return objMgr.triangleIndex(triangle);
	}
};

struct SphereIndexGenerator{
	cl_int * sprCount;
	std::vector<KDNode*> nodes;
	ObjectManager &objMgr;

	SphereIndexGenerator(cl_int * sc, std::vector<KDNode*> n, ObjectManager &om)
		:sprCount(sc), nodes(n), objMgr(om){}

	cl_int operator()(){
		static int i = 0, s = 0;
		int idx = objMgr.sphereIndex(nodes[i]->m_spheres[s]);

		++s;
		while (s >= sprCount[i]){
			++i;
			s = 0;
		}
		return idx;
	}
};

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
			m_triOffset[i] = m_triCount[i - 1] + m_triOffset[i - 1];
			m_sprOffset[i] = m_sprCount[i - 1] + m_sprOffset[i - 1];
		}
		else{
			m_triOffset[i] = 0;
			m_sprOffset[i] = 0;
		}
	}

	m_triindices = (cl_int*)malloc(sizeof(cl_int) * triIdxCnt);
	m_sprindices = (cl_int*)malloc(sizeof(cl_int) * sprIdxCnt);
	OutputDebugString("Begin to generate indices.\n");

	std::generate_n(m_triindices, triIdxCnt, TriangleIndexGenerator(m_triCount, nodes, m_objMgr));
	std::generate_n(m_sprindices, sprIdxCnt, SphereIndexGenerator(m_sprCount, nodes, m_objMgr));
	OutputDebugString("indices generated\n");

	/*for (int i = 0; i < nodeCount; ++i){
		for (int t = 0; t < m_triCount[i]; ++t){
			m_triindices[m_triOffset[i] + t] = m_objMgr.triangleIndex(nodes[i]->m_triangles[t]);
		}
	}*/

	m_buffMgr.createBuffer("positions", sizeof(cl_float3) * nodeCount, m_positions);
	m_buffMgr.createBuffer("dimensions", sizeof(cl_float3) * nodeCount, m_dimensions);
	m_buffMgr.createBuffer("axis", sizeof(cl_char) * nodeCount, m_axis);
	m_buffMgr.createBuffer("kdLeft", sizeof(cl_int) * nodeCount, m_left);
	m_buffMgr.createBuffer("kdRight", sizeof(cl_int) * nodeCount, m_right);

	m_buffMgr.createBuffer("kdTriIndices", sizeof(cl_int) * triIdxCnt, m_triindices);
	m_buffMgr.createBuffer("kdTriCount", sizeof(cl_int) * nodeCount, m_triCount);
	m_buffMgr.createBuffer("kdTriOffset", sizeof(cl_int) * nodeCount, m_triOffset);

	m_buffMgr.createBuffer("kdSprIndices", sizeof(cl_int) * sprIdxCnt, m_sprindices);
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