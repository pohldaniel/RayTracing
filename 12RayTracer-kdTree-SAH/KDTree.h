#ifndef _KDTREE_H
#define _KDTREE_H
#include "scene.h"
#include "Primitive.h"

class Scene;
class ObjectList;
class KdTreeNode;
class MManager
{
public:
	MManager();
	ObjectList* NewObjectList();
	void FreeObjectList(ObjectList* a_List);
	KdTreeNode* NewKdTreeNodePair();
private:
	ObjectList* m_OList;
	char* m_KdArray, *m_ObjArray;
	KdTreeNode* m_KdPtr;
	ObjectList* m_ObjPtr;
	int m_KdUsed;
};


// -----------------------------------------------------------
// Object list helper class
// -----------------------------------------------------------

class ObjectList
{
public:
	ObjectList() : m_Primitive(0), m_Next(0) {}
	~ObjectList() { delete m_Next; }
	void SetPrimitive(Triangle* a_Prim) { m_Primitive = a_Prim; }
	Triangle* GetPrimitive() { return m_Primitive; }
	void SetNext(ObjectList* a_Next) { m_Next = a_Next; }
	ObjectList* GetNext() { return m_Next; }
private:
	Triangle* m_Primitive;
	ObjectList* m_Next;
};

// -----------------------------------------------------------
// KdTree class definition
// -----------------------------------------------------------

class MManager;
class KdTreeNode
{
public:
	KdTreeNode() : m_Data(6) {};
	void SetAxis(int a_Axis) { m_Data = (m_Data & 0xfffffffc) + a_Axis; }
	int GetAxis() { return m_Data & 3; }
	void SetSplitPos(double a_Pos) { m_Split = a_Pos; }
	double GetSplitPos() { return m_Split; }
	void SetLeft(KdTreeNode* a_Left) { m_Data = (unsigned long)a_Left + (m_Data & 7); }
	KdTreeNode* GetLeft() { return (KdTreeNode*)(m_Data & 0xfffffff8); }
	KdTreeNode* GetRight() { return ((KdTreeNode*)(m_Data & 0xfffffff8)) + 1; }
	void Add(Triangle* a_Prim);
	bool IsLeaf() { return ((m_Data & 4) > 0); }
	void SetLeaf(bool a_Leaf) { m_Data = (a_Leaf) ? (m_Data | 4) : (m_Data & 0xfffffffb); }
	ObjectList* GetList() { return (ObjectList*)(m_Data & 0xfffffff8); }
	void SetList(ObjectList* a_List) { m_Data = (unsigned long)a_List + (m_Data & 7); }
	
private:
	double m_Split;
	unsigned long m_Data;

	
};

struct SplitList
{
	double splitpos;
	int n1count, n2count;
	SplitList* next;
};

class KdTree
{
public:
	KdTree();
	~KdTree();
	void Build(std::vector<Triangle *> T, const BBox &V);
	KdTreeNode* GetRoot() { return m_Root; }
	void SetRoot(KdTreeNode* a_Root) { m_Root = a_Root; }
	// tree generation
	void InsertSplitPos(double a_SplitPos);
	void Subdivide(KdTreeNode* a_Node, BBox& a_Box, int a_Depth, int a_Prims);
	// memory manager
	static void SetMemoryManager(MManager* a_MM) { s_MManager = a_MM; }
private:
	KdTreeNode* m_Root;
	SplitList* m_SList, *m_SPool;
public:
	static MManager* s_MManager;

};



/*struct kdstack
{
	KdTreeNode* node;
	double t;
	Vector3f pb;
	int prev, dummy1, dummy2;
};*/

#endif 