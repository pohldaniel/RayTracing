#ifndef _KDTREE_H
#define _KDTREE_H

#include <stack>
#include "scene.h"
#include "Primitive.h"


class KDTree{

public:

	struct KD_Primitive{

		KD_Primitive(Primitive* primitive){
			m_primitive = primitive;
		}

		Primitive* m_primitive;
		int		   m_orientation;
	};

	struct Node{

		Node(std::vector<KD_Primitive *> triangles, KDTree *tree){
			m_primitives = triangles;
			m_isLeaf = true;
			m_tree = tree;
		}

		Node(int splitAxis, float splitPosition){
			m_splitAxis = splitAxis;
			m_splitPosition = splitPosition;
			m_isLeaf = false;
		}

		
		int m_splitAxis;
		float m_splitPosition;
		Node* left;
		Node* right;
		bool m_isLeaf;
		std::vector<KD_Primitive*>	m_primitives;
		KDTree* m_tree;

		bool leafIntersect(const Ray& ray, Hit &hit);
		bool getNearFar(const Ray& ray, Node*& nea, Node*& fa);
		float distanceToSplitPlane(const Ray& ray);

		
	};

	struct Event{

		Event(int axis, float position, int type, KD_Primitive* primitive){
			m_axis = axis;
			m_position = position;
			m_type = type;
			m_primitive = primitive;
		}

		int m_axis;
		float m_position;
		int m_type;
		KD_Primitive* m_primitive;

		//comparison method for quicksort
		inline bool less(Event* rhs)
		{
			return m_position < rhs->m_position || (m_position == rhs->m_position && (m_axis < rhs->m_axis || (m_axis == rhs->m_axis && m_type < rhs->m_type)));
		}
	};

	KDTree();
	~KDTree();

	void buildTree(const std::vector<Triangle *>& list, const BBox &V, int maxDepth = 15);
	bool intersectRec(const Ray& ray, Hit &hit);
	
	//used for texturing the mesh
	Primitive *m_primitive;
	Triangle *m_triangle;
	

private:
	
	Node* buildTree(BBox BBox, std::vector<KD_Primitive *> primitives, std::vector<Event *> events, int depth);
	void createEvents(std::vector<Event *>& events, std::vector<KD_Primitive *>& primitives, std::vector<Triangle *> list);
	void sortEvents(std::vector<Event*>& events);
	void quickSort(std::vector<Event*>& events, unsigned int leftBorder, unsigned int rightBorder);
	void findSplitPlane(BBox boundingBox, int numberOfPrimitives, std::vector<Event*>& events, int& bestAxis, float& bestPosition, float& bestSAHValue, int& side);
	float computeSAH(BBox boundingBox, unsigned int axis, float position, unsigned int numberOfLeftPrims, unsigned int numberOfPlanarPrims, unsigned int numberOfRightPrims, unsigned int &side);
	void classifyPrimitives(std::vector<Event*>& events, std::vector<KD_Primitive*>& primitives, int axis, float position, int side);
	void spliceEvents(std::vector<Event*>& events, std::vector<Event*>& leftOnlyEvents, std::vector<Event*>& rightOnlyEvents);
	void generateNewEvents(std::vector<KD_Primitive*>& primitives, std::vector<Event*>& leftBothEvents, std::vector<Event*>& rightBothEvents, int axis, float position);
	void mergeEvents(std::vector<Event*>& finalEvents, std::vector<Event*>& primaryEvents, std::vector<Event*>& secondaryEvents);
	void splitPrimitives(std::vector<KD_Primitive*>& leftPrimitives, std::vector<KD_Primitive*>& rightPrimitives, std::vector<KD_Primitive*>& primitives);

	bool intersect(Node* node, const Ray& ray, float min, float max, Hit &hit);
	//the max depth of the tree
	int	m_maximumDepth;

	//the bounding box of the object
	BBox m_boundingBox;

	//the root node
	Node* m_rootNode;

	//the cost of intersecting a node
	int	m_costOfIntersection;

	//the cost of traversing a node
	int m_costOfTraversal;

	
};



#endif 