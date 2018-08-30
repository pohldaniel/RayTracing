#ifndef _SAMPLER_H
#define _SAMPLER_H

#include <vector>
#include "Vector.h"



class Sampler{
public:

	Sampler();
	Sampler(const int numSamples, const int numSets);
	virtual ~Sampler();
	int getNumSamples();

	virtual void generateSamples() = 0;		// generate sample patterns in a unit square
	Vector2f sampleUnitSquare();

protected:
	int						numSamples;				// the number of sample points in a set; number of rays
	int 					numSets;				// the number of sample sets
	std::vector<Vector2f>	samples;				// sample points on a unit square
	unsigned long 			count;					// the current number of sample points used
	

};


class Regular : public Sampler {
public:

	Regular();
	Regular(const int numSamples, const int numSets);
	~Regular();

	

	void generateSamples();

};

#endif