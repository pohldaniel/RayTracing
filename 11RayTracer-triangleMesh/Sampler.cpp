//debugging console
#include <iostream>

#include "Sampler.h"


Sampler::Sampler(){

	Sampler::numSamples = 4;
	Sampler::numSets = 1;
	Sampler::count = 0;

	samples.reserve(numSamples*numSets);
}

Sampler::Sampler(const int numSamples, const int numSets){

	Sampler::numSamples = numSamples;
	Sampler::numSets = numSets;
	Sampler::count = 0;
	Sampler::samples.reserve(numSamples * numSets);

}

int Sampler::getNumSamples(){

	return numSamples;
}

Sampler::~Sampler(){}

Vector2f Sampler::sampleUnitSquare(){
	
	if (count == numSamples)count = 0;

	count++;
	
	return samples[count-1];

}

//////////////////////////////////////////////////////////

Regular::Regular():Sampler(){

	generateSamples();

}

Regular::Regular(const int numSamples, const int numSets):Sampler(numSamples, numSets){
	generateSamples();
}

Regular::~Regular() {}

void Regular::generateSamples() {
	int n = (int)sqrt((float)numSamples);

	
		for (int p = 0; p < n; p++){
			for (int q = 0; q < n; q++){
				samples.push_back(Vector2f((q + 0.5) / n, (p + 0.5) / n));
				
			}
		}
}