#ifndef _KDTREE_H
#define _KDTREE_H

#include <stack>
#include "scene.h"
#include "Primitive.h"


class KDTree{

public:

	struct KD_Primitive{

		KD_Primitive(std::shared_ptr<Primitive> primitive){
			m_primitive = primitive;
		}

		std::shared_ptr<Primitive> m_primitive;
		int		   m_orientation;


	};

	struct Node{

		Node(std::vector<std::shared_ptr<KD_Primitive>> triangles, KDTree *tree){
			m_primitives = triangles;
			m_isLeaf = true;
			m_tree = std::unique_ptr<KDTree>(tree);
		}

		Node(int splitAxis, float splitPosition){
			m_splitAxis = splitAxis;
			m_splitPosition = splitPosition;
			m_isLeaf = false;
		}

		
		int m_splitAxis;
		float m_splitPosition;
		std::shared_ptr<Node> left;
		std::shared_ptr<Node> right;
		bool m_isLeaf;
		std::vector<std::shared_ptr<KD_Primitive>>	m_primitives;
		std::shared_ptr<KDTree> m_tree;

		bool leafIntersect(const Ray& ray, Hit &hit);
		bool getNearFar(const Ray& ray, std::shared_ptr<Node>& nea, std::shared_ptr<Node>& fa);
		float distanceToSplitPlane(const Ray& ray);

		
	};

	struct Event{

		Event(int axis, float position, int type, std::shared_ptr<KD_Primitive> primitive){
			m_axis = axis;
			m_position = position;
			m_type = type;
			m_primitive = primitive;
		}

		int m_axis;
		float m_position;
		int m_type;
		std::shared_ptr<KD_Primitive> m_primitive;
		
		//comparison method for quicksort
		inline bool less(std::shared_ptr<Event> rhs){

			return m_position < rhs->m_position || (m_position == rhs->m_position && (m_axis < rhs->m_axis || (m_axis == rhs->m_axis && m_type < rhs->m_type)));
		}
	};

	KDTree();
	~KDTree();
	
	void buildTree(const std::vector<std::shared_ptr<Triangle>>& list, const BBox &V, int maxDepth = 15);
	bool intersectRec(const Ray& ray, Hit &hit);
	
	// used to get the right material at the render function in the class scene
	std::shared_ptr<Primitive> m_primitive;

private:
	
	std::shared_ptr<Node> buildTree(BBox BBox, std::vector<std::shared_ptr<KD_Primitive>> primitives, std::vector<std::shared_ptr<Event>> events, int depth);
	void createEvents(std::vector<std::shared_ptr<Event>>& events, std::vector<std::shared_ptr<KD_Primitive>>& primitives, std::vector<std::shared_ptr<Triangle>> list);
	void sortEvents(std::vector<std::shared_ptr<Event>>& events);
	void quickSort(std::vector<std::shared_ptr<Event>>& events, unsigned int leftBorder, unsigned int rightBorder);
	void findSplitPlane(BBox boundingBox, int numberOfPrimitives, std::vector<std::shared_ptr<Event>>& events, int& bestAxis, float& bestPosition, float& bestSAHValue, int& side);
	float computeSAH(BBox boundingBox, unsigned int axis, float position, unsigned int numberOfLeftPrims, unsigned int numberOfPlanarPrims, unsigned int numberOfRightPrims, unsigned int &side);
	void classifyPrimitives(std::vector<std::shared_ptr<Event>>& events, std::vector<std::shared_ptr<KD_Primitive>>& primitives, int axis, float position, int side);
	void spliceEvents(std::vector<std::shared_ptr<Event>>& events, std::vector<std::shared_ptr<Event>>& leftOnlyEvents, std::vector<std::shared_ptr<Event>>& rightOnlyEvents);
	void generateNewEvents(std::vector<std::shared_ptr<KD_Primitive>>& primitives, std::vector<std::shared_ptr<Event>>& leftBothEvents, std::vector<std::shared_ptr<Event>>& rightBothEvents, int axis, float position);
	void mergeEvents(std::vector<std::shared_ptr<Event>>& finalEvents, std::vector<std::shared_ptr<Event>>& primaryEvents, std::vector<std::shared_ptr<Event>>& secondaryEvents);
	void splitPrimitives(std::vector<std::shared_ptr<KD_Primitive>>& leftPrimitives, std::vector<std::shared_ptr<KD_Primitive>>& rightPrimitives, std::vector<std::shared_ptr<KD_Primitive>>& primitives);

	bool intersect(std::shared_ptr<Node> node, const Ray& ray, float min, float max, Hit &hit);
	//the max depth of the tree
	int	m_maximumDepth;

	//the bounding box of the object
	BBox m_boundingBox;

	//the root node
	std::shared_ptr<Node> m_rootNode;

	//the cost of intersecting a node
	int	m_costOfIntersection;

	//the cost of traversing a node
	int m_costOfTraversal;

	
};



#endif 