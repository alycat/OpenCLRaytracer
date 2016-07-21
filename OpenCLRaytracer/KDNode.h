#ifndef KDNODE_H
#define KDNODE_H

#include "BoundingBox.h"
#include <vector>
#include "Triangle.h"
#include "Sphere.h"

class KDNode{
public:
	KDNode();
	KDNode(BoundingBox bbox, 
		KDNode* left = nullptr, KDNode* right = nullptr, 
		std::vector<Triangle> triangles = {}, std::vector<Sphere> spheres = {}, char axis = 0);
	KDNode(const KDNode & other);
	KDNode& operator=(const KDNode & other);
	~KDNode();
	bool operator==(const KDNode & other);

	KDNode* m_left;
	KDNode* m_right;

	BoundingBox m_bbox;
	char m_axis;

	std::vector<Triangle> m_triangles;
	std::vector<Sphere> m_spheres;
	void nodes(std::vector<KDNode*>& nodes);
protected:
private:
};

class ObjectManager;
class BufferManager;

class KDTree{
public:
	KDTree(BoundingBox bbox, ObjectManager & objMgr, BufferManager & buffMgr);
	KDTree(const KDTree& other);
	KDTree& operator=(const KDTree& other);
	~KDTree();
	std::vector<KDNode*> nodes();
	void createKDTreeBuffers();
	int nodeCount();
protected:
	KDNode* m_root;

	std::vector<Triangle> m_triangles;
	std::vector<Sphere> m_spheres;
	BoundingBox m_bbox;

	void build(KDNode* node, int depth = 0);

	cl_float3 * m_positions;
	cl_float3 * m_dimensions;

	cl_char * m_axis;
	
	cl_int * m_triindices;
	cl_int * m_triCount;
	cl_int * m_triOffset;

	cl_int * m_sprindices;
	cl_int * m_sprCount;
	cl_int * m_sprOffset;

	cl_int * m_left;
	cl_int * m_right;


	ObjectManager & m_objMgr;
	BufferManager & m_buffMgr;
private:
};

#endif