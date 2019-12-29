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
	virtual void generateSamples2();	// generate sample patterns in a unit square
	Vector2f sampleUnitSquare();
	Vector2f sampleUnitDisk();
	Vector2f sampleOneSet();
	Vector3f sampleHemisphere();
	Vector3f sampleSphere();

	void mapSamplesToUnitDisk();
	void mapSamplesToHemisphere(const float p);
	void mapSamplesToSphere();

protected:
	void shuffleXcoordinates();
	void shuffleYcoordinates();
	int randInt(int l, int h);
	float randFloat();
	float randFloat(int l, float h);

	int						m_numSamples;			// the number of sample points in a set; number of rays
	int 					m_numSets;				// the number of sample sets
	std::vector<Vector2f>	m_samples;				// sample points on a unit square
	std::vector<Vector2f>	m_samples2;				// sample points on a unit square
	std::vector<Vector2f>	m_diskSamples;			// sample points on a unit disk
	std::vector<Vector3f> 	m_hemisphereSamples;	// sample points on a unit hemisphere
	std::vector<Vector3f> 	m_sphereSamples;		// sample points on a unit sphere
	unsigned long 			m_countSquare;			// the current number of sample points used
	unsigned long 			m_countDisk;
	unsigned long 			m_countSphere;
	unsigned long 			m_countHemisphere;
	unsigned long 			m_countOneSet;
	unsigned long 			m_count2;				// the current number of sample points used
	int 					m_jump;					// random index jump
	std::vector<int>		m_shuffledIndices;		// shuffled samples array indices
	
private:

	void setupShuffledIndices();	
	
};
//////////////////////////////////////Regular////////////////////////////////////////////////////////////
class Regular : public Sampler {
public:

	Regular();
	Regular(const int numSamples, const int numSets);
	~Regular();

private:
	void generateSamples();
	void generateSamples2();
};
//////////////////////////////////////PureRandom////////////////////////////////////////////////////////////
class PureRandom : public Sampler {
public:

	PureRandom();
	PureRandom(const int numSamples, const int numSets);
	~PureRandom();

private:
	void generateSamples();

};
//////////////////////////////////////NRooks////////////////////////////////////////////////////////////
class NRooks : public Sampler {
public:

	NRooks();
	NRooks(const int numSamples, const int numSets);
	~NRooks();

private:
	void generateSamples();

};
//////////////////////////////////////MultiJittered////////////////////////////////////////////////////////////
class MultiJittered : public Sampler {
public:

	MultiJittered();
	MultiJittered(const int numSamples, const int numSets);
	~MultiJittered();

private:
	void generateSamples();

};
//////////////////////////////////////Jittered////////////////////////////////////////////////////////////
class Jittered : public Sampler {
public:

	Jittered();
	Jittered(const int numSamples, const int numSets);
	~Jittered();

private:
	void generateSamples();

};
//////////////////////////////////////Hammersley////////////////////////////////////////////////////////////
class Hammersley : public Sampler {
public:

	Hammersley();
	Hammersley(const int numSamples, const int numSets);
	~Hammersley();

private:
	double phi(int j);
	void generateSamples();

};
#endif