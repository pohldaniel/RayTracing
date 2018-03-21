#include "KDTree.h"


KDTree::KDTree(){
	m_rootNode = NULL;
	m_costOfIntersection = 80;
	m_costOfTraversal = 1;
}

//the destructor
KDTree::~KDTree(){

}

//the method that is called from extern to built the tree
void KDTree::buildTree(const std::vector<Triangle *> &list, const BBox &bbox, int maxDepth){
	//setting the needed information
	m_boundingBox = bbox;
	m_maximumDepth = maxDepth;

	//the data-structures for the sah heuristic
	std::vector<KD_Primitive*>	primitives;
	std::vector<Event*>			events;

	//building the list of events and setting the additional information to the primitives
	createEvents(events, primitives, list);

	//sorting the events one single time
	sortEvents(events);

	m_rootNode = buildTree(m_boundingBox, primitives, events, 0);

}


KDTree::Node* KDTree::buildTree(BBox boundingBox, std::vector<KD_Primitive*> primitives, std::vector<Event*> events, int depth){

	//first termination criteria: have we got some primitives to insert?
	if (primitives.size() == 0)
	{
		//create a empty leaf node
		return new Node(primitives, this);
	}

	//second termination criteria: test if maxDepth has been reached
	if (depth == m_maximumDepth)
	{
		//we have to clean up a bit
		for (int i = 0; i < events.size(); i++)
		if (events[i]) delete events[i];

		//create a leaf node with the primitives
		return new Node(primitives, this);
	}

	//the values computed by the sah function
	int	splitAxis;
	float	splitPosition;
	int	splitSide;

	//then find the next split-plane
	float SAHValue;
	findSplitPlane(boundingBox, (int)primitives.size(), events, splitAxis, splitPosition, SAHValue, splitSide);

	//third termination critera: is it useful to split??
	if (SAHValue >= m_costOfIntersection*primitives.size())
	{
		//clean up
		for (int i = 0; i < events.size(); i++)
		if (events[i]) delete events[i];

		//create a leaf node with the primitives
		return new Node(primitives, this);
	}


	//the following steps are needed to conserve the properties of the event list
	//compare: paper
	//step 1: classification of the primitives
	classifyPrimitives(events, primitives, splitAxis, splitPosition, splitSide);


	//the vectors for step 2 and 3
	std::vector<Event*> leftOnlyEvents;
	std::vector<Event*> rightOnlyEvents;
	std::vector<Event*> leftBothEvents;
	std::vector<Event*> rightBothEvents;


	//step 2: splicing the events
	spliceEvents(events, leftOnlyEvents, rightOnlyEvents);

	//step 3: generate new events
	generateNewEvents(primitives, leftBothEvents, rightBothEvents, splitAxis, splitPosition);

	sortEvents(leftBothEvents);
	sortEvents(rightBothEvents);

	std::vector<Event*> leftFinalEvents;
	std::vector<Event*> rightFinalEvents;

	//step 4: merging
	mergeEvents(leftFinalEvents, leftBothEvents, leftOnlyEvents);
	mergeEvents(rightFinalEvents, rightBothEvents, rightOnlyEvents);


	std::vector<KD_Primitive*> leftPrimitives;
	std::vector<KD_Primitive*> rightPrimitives;

	//step 5: split primitives
	splitPrimitives(leftPrimitives, rightPrimitives, primitives);

	Node* node = new Node(splitAxis, splitPosition);

	BBox leftBoundingBox = boundingBox;
	BBox rightBoundingBox = boundingBox;

	leftBoundingBox.getSize()[splitAxis] = splitPosition;
	rightBoundingBox.getPos()[splitAxis] = splitPosition;

	//clearing the vectors
	events.clear();
	primitives.clear();
	leftBothEvents.clear();
	leftOnlyEvents.clear();
	rightBothEvents.clear();
	rightOnlyEvents.clear();

	node->left = buildTree(leftBoundingBox, leftPrimitives, leftFinalEvents, depth + 1);
	node->right = buildTree(rightBoundingBox, rightPrimitives, rightFinalEvents, depth + 1);

	return node;
}

void KDTree::createEvents(std::vector<Event *>& events, std::vector<KD_Primitive *>& primitives, std::vector<Triangle *> list){

	Primitive* primitive = NULL;

	for (int i = 0; i < list.size(); i++){

		//take the next one
		primitive = list[i];

		//and make it a kdprimitive
		KD_Primitive* kdPrimitive = new KD_Primitive(primitive);

		//and store it
		primitives.push_back(kdPrimitive);

		//we need the borders of the primitive
		BBox primBounds = primitive->getBounds();

		//for each dimension
		for (int j = 0; j < 3; j++)
		{
			//look at the bounds in the given dimension
			float min = primBounds.getPos()[j]  ;
			float max = primBounds.getSize()[j] ;


			//if they are the same, the primitive is perpendicular to that dimension
			if (min == max)
			{
				Event* planarEvent = new Event(j, min, 1, kdPrimitive);
				events.push_back(planarEvent);
			}
			else
			{
				Event* endEvent = new Event(j, max, 0, kdPrimitive);
				Event* startEvent = new Event(j, min, 2, kdPrimitive);
				events.push_back(endEvent);
				events.push_back(startEvent);
			}
		}
	}

	
}

void KDTree::sortEvents(std::vector<Event*>& events){

	quickSort(events, 0, (int)events.size() - 1);
}


void KDTree::quickSort(std::vector<Event*>& events, unsigned int leftBorder, unsigned int rightBorder){
	std::stack<std::pair<int, int> > indices;

	indices.push(std::make_pair((int)leftBorder, (int)rightBorder));

	std::pair<int, int> currentBorders;
	int newLeftBorder; int newRightBorder;

	int a = 5;

	while (!indices.empty())
	{
		currentBorders = indices.top();
		indices.pop();
		if (currentBorders.first < currentBorders.second)
		{
			newLeftBorder = currentBorders.first;
			newRightBorder = currentBorders.second;
			int index = newLeftBorder;

			Event* helpEvent;
			Event* pivot = events[currentBorders.first];

			while (index <= newRightBorder)
			{
				if (events[index]->less(pivot))
				{
					helpEvent = events[newLeftBorder];
					events[newLeftBorder++] = events[index];
					events[index++] = helpEvent;
				}
				else if (pivot->less(events[index]))
				{
					helpEvent = events[newRightBorder];
					events[newRightBorder--] = events[index];
					events[index] = helpEvent;
				}
				else index++;
			}

			indices.push(std::make_pair(newRightBorder + 1, currentBorders.second));
			indices.push(std::make_pair(currentBorders.first, newLeftBorder - 1));
		}
	}
}

void KDTree::findSplitPlane(BBox boundingBox, int numberOfPrimitives, std::vector<Event*>& events, int& bestAxis, float& bestPosition, float& bestSAHValue, int& side){
	//we need to count the number of primitives on the left, on the plane itself and on the right side, and this for each dimension
	//these values are stored here
	int leftCount[3];
	int planarCount[3];
	int rightCount[3];

	//init these values
	for (int i = 0; i < 3; i++)
	{
		//at the beginning of the sweeping there are no prims on the left and on the plane
		leftCount[i] = 0;
		planarCount[i] = 0;
		//but all are to the right
		rightCount[i] = numberOfPrimitives;
	}

	//trying to find better values, so start with the maximum
	bestSAHValue = FLT_MAX;

	int index = 0;

	//testing for each eventposition if we found a better sah value
	while (index < events.size())
	{
		int currentAxis = events[index]->m_axis;
		float currentPosition = events[index]->m_position;

		int currentEndings = 0;
		int currentPlanars = 0;
		int currentStarts = 0;

		//intercept the cases where several events got the same position on the same axis
		while (index < events.size() && events[index]->m_position == currentPosition && events[index]->m_axis == currentAxis && events[index]->m_type == 0)
		{
			//look at the next event
			index++;
			//and we found an end event
			currentEndings++;
		}
		//the same for type 1 and 2
		while (index < events.size() && events[index]->m_position == currentPosition && events[index]->m_axis == currentAxis && events[index]->m_type == 1)
		{
			//look at the next event
			index++;
			//and we found an planar event
			currentPlanars++;
		}
		while (index < events.size() && events[index]->m_position == currentPosition && events[index]->m_axis == currentAxis && events[index]->m_type == 2)
		{
			//look at the next event
			index++;
			//and we found an start event
			currentStarts++;
		}

		//now we can compute the wanted numbers
		//the number of primitives in the plane equals the number of planar events for this location
		planarCount[currentAxis] = currentPlanars;
		//the number of primitives on the right is the same as before minus those who ended in the plane and minus those who are in the plane
		rightCount[currentAxis] -= (currentPlanars + currentEndings);

		//looking for the sah value of this configuration
		//version 2.0: look only in not flat boundingBoxes
		if (boundingBox.getPos()[currentAxis] < boundingBox.getSize()[currentAxis])
		{
			unsigned int newSide;

			//compute the sah value
			float newSAHValue = computeSAH(boundingBox, currentAxis, currentPosition, leftCount[currentAxis], planarCount[currentAxis], rightCount[currentAxis], newSide);

			//better than the last one?
			if (newSAHValue < bestSAHValue)
			{
				side = newSide;
				bestAxis = currentAxis;
				bestPosition = currentPosition;
				bestSAHValue = newSAHValue;
			}
		}

		//finally we have to compute the numbers for the next event position
		leftCount[currentAxis] += currentPlanars + currentStarts;
	}
}

float KDTree::computeSAH(BBox boundingBox, unsigned int axis, float position, unsigned int numberOfLeftPrims, unsigned int numberOfPlanarPrims, unsigned int numberOfRightPrims, unsigned int &side){
	
	//we need the areas of the left and right side, so make two new boxes
	BBox leftBox = boundingBox;
	BBox rightBox = boundingBox;

	//and split them
	leftBox.getSize()[axis] = position;
	rightBox.getPos()[axis] = position;

	//the boxes compute the areas
	float leftArea = leftBox.getSurfaceArea();
	float rightArea = rightBox.getSurfaceArea();
	float overallArea = boundingBox.getSurfaceArea();

	//compute the parts of left and right
	leftArea /= overallArea;
	rightArea /= overallArea;

	//now look at the sah value (note: we get two values since we do not know where to put the primitives in the plane)
	float leftSAHValue = m_costOfTraversal + m_costOfIntersection * (leftArea * (numberOfLeftPrims + numberOfPlanarPrims) + rightArea * numberOfRightPrims);
	float rightSAHValue = m_costOfTraversal + m_costOfIntersection * (leftArea * numberOfLeftPrims + rightArea * (numberOfPlanarPrims + numberOfRightPrims));

	//as proposed in the paper: put a bonus if we generate empty boxes
	if (leftBox.getPos()[axis] < leftBox.getSize()[axis])
	{
		//we can put the plane prims to the left and generate an empty box this way, so the leftValue has to be pushed
		if (numberOfLeftPrims + numberOfPlanarPrims == 0)
			leftSAHValue *= 0.8f;
		//we can also put the planar prims to the right and generate an empty box, so the rightValue can be pushed
		if (numberOfLeftPrims == 0)
			rightSAHValue *= 0.8f;
	}
	//the same for the other side
	if (rightBox.getPos()[axis] < rightBox.getSize()[axis])
	{
		if (numberOfRightPrims == 0)
			leftSAHValue *= 0.8f;
		if (numberOfPlanarPrims + numberOfRightPrims == 0)
			rightSAHValue *= 0.8f;
	}

	//finally compare the values
	if (leftSAHValue <= rightSAHValue)
	{
		//put the plane prims to the left
		side = 0;
		return leftSAHValue;
	}
	//or to the right
	side = 1;
	return rightSAHValue;
}


void KDTree::classifyPrimitives(std::vector<Event*>& events, std::vector<KD_Primitive*>& primitives, int axis, float position, int side)
{
	//first set each primitive to both sides
	for (int i = 0; i < primitives.size(); i++)
	{
		primitives[i]->m_orientation = 1;
	}

	//then look at each event and set the primitives
	for (int i = 0; i < events.size(); i++)
	{
		//first test if the primitive of the event lies entirely on the left
		//this happens if the location of the event lies on the left and the event is end
		if (events[i]->m_position <= position && events[i]->m_axis == axis && events[i]->m_type == 0)
		{
			//put the primitive to the left
			events[i]->m_primitive->m_orientation = 0;
		}
		//same for the right side with start events
		else if (events[i]->m_position >= position && events[i]->m_axis == axis && events[i]->m_type == 2)
		{
			//put the primitive to the right
			events[i]->m_primitive->m_orientation = 2;
		}
		//finally look at the planar events
		else if (events[i]->m_axis == axis && events[i]->m_type == 1)
		{
			if (events[i]->m_position < position || (events[i]->m_position == position && side == 0))
			{
				//put the primitive to the left
				events[i]->m_primitive->m_orientation = 0;
			}
			if (events[i]->m_position > position || (events[i]->m_position == position && side == 1))
			{
				//put the primitive to the right
				events[i]->m_primitive->m_orientation = 2;
			}
		}
	}
}

void KDTree::spliceEvents(std::vector<Event*> &events, std::vector<Event*> &leftOnlyEvents, std::vector<Event*> &rightOnlyEvents)
{
	//run over the events and put them into the corresponding list
	for (int i = 0; i < events.size(); i++)
	{
		switch (events[i]->m_primitive->m_orientation)
		{
		case 0:			leftOnlyEvents.push_back(events[i]);
			break;
		case 1:			delete events[i];
			break;
		case 2:			rightOnlyEvents.push_back(events[i]);
			break;
		}
	}
}

void KDTree::generateNewEvents(std::vector<KD_Primitive*> &primitives, std::vector<Event*> &leftBothEvents, std::vector<Event*> &rightBothEvents, int axis, float position)
{
	for (int i = 0; i < primitives.size(); i++)
	{
		if (primitives[i]->m_orientation == 1)
		{
			BBox left, right;
			primitives[i]->m_primitive->clip(axis, position , left, right);

			//for each dimension
			for (int j = 0; j < 3; j++)
			{

				float minLeft = left.getPos()[j] ;
				float maxLeft = left.getSize()[j];
				float minRight = right.getPos()[j];
				float maxRight = right.getSize()[j];

				
				//if they are the same, the primitive is perpendicular to that dimension
				if (minLeft == maxLeft)
				{
					Event* planarEvent = new Event(j, minLeft, 1, primitives[i]);
					leftBothEvents.push_back(planarEvent);
				}
				else
				{
					Event* endEvent = new Event(j, maxLeft, 0, primitives[i]);
					Event* startEvent = new Event(j, minLeft, 2, primitives[i]);
					leftBothEvents.push_back(endEvent);
					leftBothEvents.push_back(startEvent);
				}

				if (minRight == maxRight)
				{
					Event* planarEvent = new Event(j, minRight, 1, primitives[i]);
					rightBothEvents.push_back(planarEvent);
				}
				else
				{
					Event* endEvent = new Event(j, maxRight, 0, primitives[i]);
					Event* startEvent = new Event(j, minRight, 2, primitives[i]);
					rightBothEvents.push_back(endEvent);
					rightBothEvents.push_back(startEvent);
				}
			}
		}
	}
}

void KDTree::mergeEvents(std::vector<Event*> &finalEvents, std::vector<Event*> &primaryEvents, std::vector<Event*> &secondaryEvents)
{
	int k = 0, l = 0;

	//run over the primitives until one list has been processed
	while (k < primaryEvents.size() && l < secondaryEvents.size())
	{
		if (primaryEvents[k]->less(secondaryEvents[l]))
		{
			finalEvents.push_back(primaryEvents[k]);
			k++;
		}
		else
		{
			finalEvents.push_back(secondaryEvents[l]);
			l++;
		}
	}
	//then process the other to the end
	while (k < primaryEvents.size())
	{
		finalEvents.push_back(primaryEvents[k]);
		k++;
	}
	while (l < secondaryEvents.size())
	{
		finalEvents.push_back(secondaryEvents[l]);
		l++;
	}
}


void KDTree::splitPrimitives(std::vector<KD_Primitive*>& leftPrimitives, std::vector<KD_Primitive*>& rightPrimitives, std::vector<KD_Primitive*>& primitives)
{
	//running over the primitives and put them into the corresponding lists
	for (int i = 0; i < primitives.size(); i++)
	{
		switch (primitives[i]->m_orientation)
		{
		case 0:				leftPrimitives.push_back(primitives[i]);
			break;
		case 1:				leftPrimitives.push_back(primitives[i]);
			rightPrimitives.push_back(primitives[i]);
			break;
		case 2:				rightPrimitives.push_back(primitives[i]);
			break;
		}
	}
}



bool KDTree::intersectRec(const Ray& ray, Hit &hit)
{
	// intersect the ray with the bounding box of the kdtree
	
	
	
	if (!m_boundingBox.intersect(ray)){
		//hit.color = Color(1.0, 0.0, 0.0);
		hit.hitObject = false;
		return false;
	}

	float tmin = m_boundingBox.m_tmin;
	float tmax = m_boundingBox.m_tmax;

	// start traversal from the root node
	return intersect(m_rootNode, ray, tmin - fabsf(tmin * 0.00001f), tmax, hit);
}


bool KDTree::intersect(Node* node, const Ray& ray, float min, float max, Hit &hit){
	
	// don't do anything for null nodes
	if (node == NULL){ 
		
		return false; }

	// if leaf node, then look for intersection with primitives
	if (node->m_isLeaf)
	{
		

		return node->leafIntersect(ray, hit);
	}

	// get near and far child
	Node* nea = NULL;
	Node* fa = NULL;
	node->getNearFar(ray, nea, fa);

	// compute distance to the split plane
	float dist = node->distanceToSplitPlane(ray);

	// the whole interval is on near side

	if (dist < 0 || dist > max + 1E-4f)
	{
		return intersect(nea, ray, min, max, hit);

		// whole interval is on far side
	}
	else if (dist <= min - 1E-5f)
	{
		return intersect(fa, ray, min, max, hit);

		// the interval intersects the plane
	}
	else
	{

		// first test near side
		if (intersect(nea, ray, min, dist, hit)) return true;
		
		// then test the far side
		return intersect(fa, ray, dist, max, hit);
	}
	return false;
}




bool KDTree::Node::leafIntersect(const Ray& ray, Hit &hit){
	
	float tmin = hit.t;
	float tmin2 = hit.t;

	
	Hit tmp;

		for (unsigned int i = 0; i < m_primitives.size(); i++){
			m_primitives[i]->m_primitive->hit(ray, tmp);

			
			if (tmp.hitObject && tmp.t < tmin2) {

				m_tree->m_primitive = m_primitives[i]->m_primitive;
				tmp.color = m_primitives[i]->m_primitive->m_color;
				tmin2 = tmp.t;
				
			}

		}
		
		// find closest triangle
		if (tmin2 <= tmin){

			//hit.color = Color(1.0,0.0,1.0);
			hit.t = tmin2;
			hit.hitObject = tmp.hitObject;
		}

		return tmp.hitObject;

	

	
	
	

	
}

bool KDTree::Node::getNearFar(const Ray &r, KDTree::Node *&nea, KDTree::Node *&fa){
	if (m_splitPosition >= r.origin[m_splitAxis])
	{
		nea = left;
		fa = right;
	}
	else
	{
		nea = right;
		fa = left;
	}
	return true;
}

float KDTree::Node::distanceToSplitPlane(const Ray &ray){

	return (m_splitPosition - ray.origin[m_splitAxis]) / ray.direction[m_splitAxis];
}