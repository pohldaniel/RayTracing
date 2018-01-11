// -----------------------------------------------------------
// memory.h
// 2004 - Jacco Bikker - jacco@bik5.com - www.bik5.com -   <><
// -----------------------------------------------------------


#include "KDTree.h"

MManager* KdTree::s_MManager = 0;

MManager::MManager() : m_OList(0)
{
	// build a 32-byte aligned array of KdTreeNodes
	m_KdArray = (char*)(new KdTreeNode[1000000]);
	m_ObjArray = (char*)(new ObjectList[100000]);
	unsigned long addr = (unsigned long)m_KdArray;
	m_KdPtr = (KdTreeNode*)((addr + 32) & (0xffffffff - 31));
	addr = (unsigned long)m_ObjArray;
	m_ObjPtr = (ObjectList*)((addr + 32) & (0xffffffff - 31));
	ObjectList* ptr = m_ObjPtr;
	for (int i = 0; i < 99995; i++)
	{
		ptr->SetNext(ptr + 1);
		ptr++;
	}
	ptr->SetNext(0);
	m_OList = m_ObjPtr;
}

ObjectList* MManager::NewObjectList()
{
	ObjectList* retval;
	retval = m_OList;
	m_OList = m_OList->GetNext();
	retval->SetNext(0);
	retval->SetPrimitive(0);
	return retval;
}

void MManager::FreeObjectList(ObjectList* a_List)
{
	ObjectList* list = a_List;
	while (list->GetNext()) list = list->GetNext();
	list->SetNext(m_OList);
	m_OList = a_List;
}

KdTreeNode* MManager::NewKdTreeNodePair()
{
	unsigned long* tmp = (unsigned long*)m_KdPtr;
	tmp[1] = tmp[3] = 6;
	KdTreeNode* node = m_KdPtr;
	m_KdPtr += 2;
	return node;
}


KdTree::KdTree(){
	m_Root = new KdTreeNode();
	
}
KdTree::~KdTree(){
	delete s_MManager;
}


void KdTree::Build(std::vector<Triangle *> triangles, const BBox &bBox){
	SetMemoryManager(new MManager());

	int prims = triangles.size();

	for (int i = 0; i < prims; i++){
		m_Root->Add(triangles[i]);
	}
	
	BBox box = bBox;
	m_SPool = new SplitList[prims * 2 + 8];
	for (int i = 0; i < (prims * 2 + 6); i++){ m_SPool[i].next = &m_SPool[i + 1]; }

	m_SList = 0;
	Subdivide(m_Root, box, 0, prims);
}


void KdTree::InsertSplitPos(double a_SplitPos)
{
	// insert a split position candidate in the list if unique
	SplitList* entry = m_SPool;
	m_SPool = m_SPool->next;
	entry->next = 0;
	entry->splitpos = a_SplitPos;
	entry->n1count = 0;
	entry->n2count = 0;
	if (!m_SList) m_SList = entry; else
	{
		if (a_SplitPos < m_SList->splitpos)
		{
			entry->next = m_SList;
			m_SList = entry;
		}
		else if (a_SplitPos == m_SList->splitpos)
		{
			entry->next = m_SPool; // redundant; recycle
			m_SPool = entry;
		}
		else
		{
			SplitList* list = m_SList;
			while ((list->next) && (a_SplitPos >= list->next->splitpos))
			{
				if (a_SplitPos == list->next->splitpos)
				{
					entry->next = m_SPool; // redundant; recycle
					m_SPool = entry;
					return;
				}
				list = list->next;
			}
			entry->next = list->next;
			list->next = entry;
		}
	}
}




void KdTree::Subdivide(KdTreeNode* a_Node, BBox& a_Box, int a_Depth, int a_Prims)
{
	
	// recycle used split list nodes
	if (m_SList)
	{
		SplitList* list = m_SList;
		while (list->next) list = list->next;
		list->next = m_SPool;
		m_SPool = m_SList, m_SList = 0;
	}
	// determine split axis
	Vector3f s = a_Box.GetSize();
	if ((s[0] >= s[1]) && (s[0] >= s[2])) a_Node->SetAxis(0);
	else if ((s[1] >= s[0]) && (s[1] >= s[2])) a_Node->SetAxis(1);
	int axis = a_Node->GetAxis();
	// make a list of the split position candidates
	ObjectList* l = a_Node->GetList();
	double p1, p2;
	double pos1 = a_Box.GetPos()[axis];
	double pos2 = a_Box.GetPos()[axis] + a_Box.GetSize()[axis];
	bool* pright = new bool[a_Prims];
	double* eleft = new double[a_Prims], *eright = new double[a_Prims];
	Triangle** parray = new Triangle*[a_Prims];
	int aidx = 0;
	while (l)
	{
		Triangle* p = parray[aidx] = l->GetPrimitive();
		pright[aidx] = true;
		p->CalculateRange(eleft[aidx], eright[aidx], axis);
		aidx++;
		/*if (p->GetType() == Primitive::SPHERE)
		{
			p1 = p->GetCentre()[axis] - p->GetRadius();
			p2 = p->GetCentre()[axis] + p->GetRadius();
			if ((p1 >= pos1) && (p1 <= pos2)) InsertSplitPos(p1);
			if ((p2 >= pos1) && (p2 <= pos2)) InsertSplitPos(p2);
		}*/
		//else
		//{
			for (int i = 0; i < 3; i++)
			{
				p1 = p->GetPos(i)[axis];
				if ((p1 >= pos1) && (p1 <= pos2)) InsertSplitPos(p1);
			}
		//}
		l = l->GetNext();
	}
	// determine n1count / n2count for each split position
	BBox b1, b2, b3 = a_Box, b4 = a_Box;
	SplitList* splist = m_SList;
	float b3p1 = b3.GetPos()[axis];
	float b4p2 = b4.GetPos()[axis] + b4.GetSize()[axis];
	while (splist)
	{
		b4.GetPos()[axis] = splist->splitpos;
		b4.GetSize()[axis] = pos2 - splist->splitpos;
		b3.GetSize()[axis] = splist->splitpos - pos1;
		float b3p2 = b3.GetPos()[axis] + b3.GetSize()[axis];
		float b4p1 = b4.GetPos()[axis];
		for (int i = 0; i < a_Prims; i++) if (pright[i])
		{
			Triangle* p = parray[i];
			if ((eleft[i] <= b3p2) && (eright[i] >= b3p1))
			if (p->IntersectBox(b3)) splist->n1count++;
			if ((eleft[i] <= b4p2) && (eright[i] >= b4p1))
			if (p->IntersectBox(b4)) splist->n2count++; else pright[i] = false;
		}
		else splist->n1count++;
		splist = splist->next;
	}
	delete pright;
	// calculate surface area for current node
	double SAV = 0.5f / (a_Box.w() * a_Box.d() + a_Box.w() * a_Box.h() + a_Box.d() * a_Box.h());
	// calculate cost for not splitting
	double Cleaf = a_Prims * 1.0f;
	// determine optimal split plane position
	splist = m_SList;
	double lowcost = 10000;
	double bestpos = 0;
	while (splist)
	{
		// calculate child node extends
		b4.GetPos()[axis] = splist->splitpos;
		b4.GetSize()[axis] = pos2 - splist->splitpos;
		b3.GetSize()[axis] = splist->splitpos - pos1;
		// calculate child node cost
		double SA1 = 2 * (b3.w() * b3.d() + b3.w() * b3.h() + b3.d() * b3.h());
		double SA2 = 2 * (b4.w() * b4.d() + b4.w() * b4.h() + b4.d() * b4.h());
		double splitcost = 0.3f + 1.0f * (SA1 * SAV * splist->n1count + SA2 * SAV * splist->n2count);
		// update best cost tracking variables
		if (splitcost < lowcost)
		{
			lowcost = splitcost;
			bestpos = splist->splitpos;
			b1 = b3, b2 = b4;
		}
		splist = splist->next;
	}
	if (lowcost > Cleaf) return;
	a_Node->SetSplitPos(bestpos);
	// construct child nodes
	KdTreeNode* left = KdTree::s_MManager->NewKdTreeNodePair();
	int n1count = 0, n2count = 0, total = 0;
	// assign primitives to both sides
	float b1p1 = b1.GetPos()[axis];
	float b2p2 = b2.GetPos()[axis] + b2.GetSize()[axis];
	float b1p2 = b1.GetPos()[axis] + b1.GetSize()[axis];
	float b2p1 = b2.GetPos()[axis];
	for (int i = 0; i < a_Prims; i++)
	{
		Triangle* p = parray[i];
		total++;
		if ((eleft[i] <= b1p2) && (eright[i] >= b1p1)) if (p->IntersectBox(b1))
		{
			left->Add(p);
			n1count++;
		}
		if ((eleft[i] <= b2p2) && (eright[i] >= b2p1)) if (p->IntersectBox(b2))
		{
			(left + 1)->Add(p);
			n2count++;
		}
	}
	delete eleft;
	delete eright;
	delete parray;
	KdTree::s_MManager->FreeObjectList(a_Node->GetList());
	a_Node->SetLeft(left);
	a_Node->SetLeaf(false);
	if (a_Depth < 20)
	{
		if (n1count > 2) Subdivide(left, b1, a_Depth + 1, n1count);
		if (n2count > 2) Subdivide(left + 1, b2, a_Depth + 1, n2count);
	}
}

// -----------------------------------------------------------
// KdTreeNode class implementation
// -----------------------------------------------------------

void KdTreeNode::Add(Triangle* a_Prim)
{
	ObjectList* lnode = KdTree::s_MManager->NewObjectList();
	lnode->SetPrimitive(a_Prim);
	lnode->SetNext(GetList());
	SetList(lnode);
}

