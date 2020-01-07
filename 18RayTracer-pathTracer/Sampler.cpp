//debugging console
#include <iostream>
#include <ctime>
#include <random>
#include "Sampler.h"


Sampler::Sampler(){

	m_numSamples = 4;
	m_numSets = 83;
	m_countSquare = 0;
	m_countDisk = 0;
	m_countSphere = 0;
	m_countHemisphere = 0;
	m_countOneSet = 0;
	m_count2 = 0;
	m_jump = 0;

	m_samples.reserve(m_numSamples * m_numSets);
	

	std::srand(std::time(NULL));
	setupShuffledIndices();
}

Sampler::Sampler(const int numSamples, const int numSets){

	m_numSamples = numSamples;
	m_numSets = numSets;
	m_countSquare = 0;
	m_countDisk = 0;
	m_countSphere = 0;
	m_countHemisphere = 0;
	m_countOneSet = 0;
	m_count2 = 0;
	m_jump = 0;

	m_samples.reserve(m_numSamples * m_numSets);
	

	std::srand(std::time(NULL));
	setupShuffledIndices();
}

Sampler::~Sampler(){}

int Sampler::getNumSamples(){

	return m_numSamples;
}

void Sampler::setupShuffledIndices(){

	m_shuffledIndices.reserve(m_numSamples * m_numSets);
	std::vector<int> indices;

	for (int j = 0; j < m_numSamples; j++){
		indices.push_back(j);
	}

	for (int p = 0; p < m_numSets; p++) {
		random_shuffle(indices.begin(), indices.end());

		for (int j = 0; j < m_numSamples; j++)
			m_shuffledIndices.push_back(indices[j]);
	}
}

void Sampler::shuffleXcoordinates() {
	for (int p = 0; p < m_numSets; p++)
	for (int i = 0; i < m_numSamples - 1; i++) {
		int target = rand() % m_numSamples + p * m_numSamples;
		float temp = m_samples[i + p * m_numSamples + 1][0];
		m_samples[i + p * m_numSamples + 1][0] = m_samples[target][0];
		m_samples[target][0] = temp;
	}
}

void Sampler::shuffleYcoordinates() {
	for (int p = 0; p < m_numSets; p++)
	for (int i = 0; i < m_numSamples - 1; i++) {
		int target = rand() % m_numSamples + p * m_numSamples;
		float temp = m_samples[i + p * m_numSamples + 1][1];
		m_samples[i + p * m_numSamples + 1][1] = m_samples[target][1];
		m_samples[target][1] = temp;
	}
}

Vector2f Sampler::sampleUnitSquare(){
	
	if (m_countSquare % m_numSamples == 0)  								// start of a new pixel
		m_jump = (rand() % m_numSets) * m_numSamples;				// random index jump initialised to zero in constructor

	//Vector2f tmp = (m_samples[m_jump + m_shuffledIndices[m_jump + m_count++ % m_numSamples]]);

	return (m_samples[m_jump + m_shuffledIndices[m_jump + m_countSquare++ % m_numSamples]]);
}

Vector2f Sampler::sampleOneSet(void) {
	return(m_samples[m_countOneSet++ % m_numSamples]);
}

Vector2f Sampler::sampleUnitDisk() {
	if (m_countDisk % m_numSamples == 0)
		m_jump = (rand() % m_numSets) * m_numSamples;

	Vector2f tmp = (m_diskSamples[m_jump + m_shuffledIndices[m_jump + m_countDisk++ % m_numSamples]]);

	//std::cout << tmp[0] << "  " << tmp[1] << std::endl;

	return tmp;
}

Vector3f Sampler::sampleHemisphere() {
	if (m_countHemisphere % m_numSamples == 0)  									// start of a new pixel
		m_jump = (rand() % m_numSets) * m_numSamples;

	return (m_hemisphereSamples[m_jump + m_shuffledIndices[m_jump + m_countHemisphere++ % m_numSamples]]);
}

Vector3f Sampler::sampleSphere() {
	if (m_countSphere % m_numSamples == 0)  									// start of a new pixel
		m_jump = (rand() % m_numSets) * m_numSamples;

	return (m_sphereSamples[m_jump + m_shuffledIndices[m_jump + m_countSphere++ % m_numSamples]]);
}

void Sampler::mapSamplesToUnitDisk() {
	int size = m_samples.size();
	float r, phi;		// polar coordinates
	Vector2f sp; 		// sample point on unit disk

	m_diskSamples.reserve(size);

	for (int j = 0; j < size; j++) {
		// map sample point to [-1, 1] X [-1,1]

		sp[0] = 2.0 * m_samples[j][0] - 1.0;
		sp[1] = 2.0 * m_samples[j][1] - 1.0;

		if (sp[0] > -sp[1]) {			// sectors 1 and 2
			if (sp[0] > sp[1]) {		// sector 1
				r = sp[0];
				phi = sp[1] / sp[0];
			}else {					// sector 2
				r = sp[1];
				phi = 2 - sp[0] / sp[1];
			}
		}else {						// sectors 3 and 4
			if (sp[0] < sp[1]) {		// sector 3
				r = -sp[0];
				phi = 4 + sp[1] / sp[0];
			}else {					// sector 4
				r = -sp[1];
				if (sp[1] != 0.0)	// avoid division by zero at origin
					phi = 6 - sp[0] / sp[1];
				else
					phi = 0.0;
			}
		}

		phi *= PI / 4.0;

		m_diskSamples[j][0] = r * cos(phi);
		m_diskSamples[j][1] = r * sin(phi);
	}

	m_samples.erase(m_samples.begin(), m_samples.end());
}

void Sampler::mapSamplesToHemisphere(const float exp) {
	int size = m_samples.size();
	m_hemisphereSamples.reserve(m_numSamples * m_numSets);

	for (int j = 0; j < size; j++) {
		float cos_phi = cos(2.0 * PI * m_samples[j][0]);
		float sin_phi = sin(2.0 * PI * m_samples[j][0]);
		float cos_theta = pow((1.0 - m_samples[j][1]), 1.0 / (exp + 1.0));
		float sin_theta = sqrt(1.0 - cos_theta * cos_theta);
		float pu = sin_theta * cos_phi;
		float pv = sin_theta * sin_phi;
		float pw = cos_theta;
		m_hemisphereSamples.push_back(Vector3f(pu, pv, pw));
	}
}

void Sampler::mapSamplesToSphere() {
	float r1, r2;
	float x, y, z;
	float r, phi;
	m_sphereSamples.reserve(m_numSamples * m_numSets);

	for (int j = 0; j < m_numSamples * m_numSets; j++) {
		r1 = m_samples[j][1];
		r2 = m_samples[j][2];
		z = 1.0 - 2.0 * r1;
		r = sqrt(1.0 - z * z);
		phi = TWO_PI * r2;
		x = r * cos(phi);
		y = r * sin(phi);
		m_sphereSamples.push_back(Vector3f(x, y, z));
	}
}

float Sampler::randFloat(){

	return (float)rand() * (1.0 / (float)RAND_MAX);
}

float Sampler::randFloat(int l, float h){
	return (randFloat() * (h - l) + l);
}

int Sampler::randInt(int l, int h){
	return ((int)(randFloat(0, h - l + 1) + l));
}

void Sampler::generateSamples2(){}
//////////////////////////////////////Regular////////////////////////////////////////////////////////////
Regular::Regular() : Sampler(){
	generateSamples();
	generateSamples2();
}

Regular::Regular(const int numSamples, const int numSets):Sampler(numSamples, numSets){
	generateSamples();
	generateSamples2();
}

Regular::~Regular() {}

void Regular::generateSamples() {
	int n = (int)sqrt((float)m_numSamples);

	for (int j = 0; j < m_numSets; j++){
		for (int p = 0; p < n; p++){
			for (int q = 0; q < n; q++){
				m_samples.push_back(Vector2f((q + 0.5f) / n, (p + 0.5f) / n));

			}
		}
	}
}

void Regular::generateSamples2() {
	int n = (int)sqrt((float)m_numSamples);

		for (int p = 0; p < n; p++){
			for (int q = 0; q < n; q++){
				m_samples2.push_back(Vector2f((q + 0.5f) / n, (p + 0.5f) / n));

			}
		}
}
//////////////////////////////////////PureRandom////////////////////////////////////////////////////////////
PureRandom::PureRandom() : Sampler(){
	generateSamples();

}

PureRandom::PureRandom(const int numSamples, const int numSets) : Sampler(numSamples, numSets){
	generateSamples();
}

PureRandom::~PureRandom() {}

void PureRandom::generateSamples() {

	/*std::random_device rd1;
	std::default_random_engine generator1(rd1());
	std::uniform_real_distribution<> dis1(0.0, 1.0);

	std::random_device rd2;
	std::default_random_engine generator2(rd2());
	std::uniform_real_distribution<> dis2(0.0, 1.0);

	for (int p = 0; p < m_numSets; p++){
		for (int q = 0; q < m_numSamples; q++){
			m_samples.push_back(Vector2f(dis1(generator1), dis2(generator2)));
		}
	}*/

	for (int p = 0; p < m_numSets; p++){
		for (int q = 0; q < m_numSamples; q++){
			m_samples.push_back(Vector2f(randFloat(), randFloat()));
		}
	}
}
//////////////////////////////////////NRooks////////////////////////////////////////////////////////////
NRooks::NRooks() : Sampler(){
	generateSamples();

}

NRooks::NRooks(const int numSamples, const int numSets) : Sampler(numSamples, numSets){
	generateSamples();
}

NRooks::~NRooks() {}


void NRooks::generateSamples() {

	for (int p = 0; p < m_numSets; p++)
	for (int j = 0; j < m_numSamples; j++) {
		Vector2f sp((j + randFloat()) / m_numSamples, (j + randFloat()) / m_numSamples);
		m_samples.push_back(sp);
	}

	shuffleXcoordinates();
	shuffleYcoordinates();
}
//////////////////////////////////////MultiJittered////////////////////////////////////////////////////////////
MultiJittered::MultiJittered() : Sampler(){
	generateSamples();

}

MultiJittered::MultiJittered(const int numSamples, const int numSets) : Sampler(numSamples, numSets){
	generateSamples();
}

MultiJittered::~MultiJittered() {}


void MultiJittered::generateSamples() {

	int n = (int)sqrt((float)m_numSamples);
	float subcell_width = 1.0 / ((float)m_numSamples);

	// fill the samples array with dummy points to allow us to use the [ ] notation when we set the 
	// initial patterns

	Vector2f fill_point;
	for (int j = 0; j < m_numSamples * m_numSets; j++)
		m_samples.push_back(fill_point);

	// distribute points in the initial patterns

	for (int p = 0; p < m_numSets; p++)
	for (int i = 0; i < n; i++)
	for (int j = 0; j < n; j++) {
		m_samples[i * n + j + p * m_numSamples][0] = (i * n + j) * subcell_width + randFloat(0, subcell_width);
		m_samples[i * n + j + p * m_numSamples][1] = (j * n + i) * subcell_width + randFloat(0, subcell_width);
	}

	// shuffle x coordinates

	for (int p = 0; p < m_numSets; p++)
	for (int i = 0; i < n; i++)
	for (int j = 0; j < n; j++) {
		int k = randInt(j, n - 1);
		float t = m_samples[i * n + j + p * m_numSamples][0];
		m_samples[i * n + j + p * m_numSamples][0] = m_samples[i * n + k + p * m_numSamples][0];
		m_samples[i * n + k + p * m_numSamples][0] = t;
	}

	// shuffle y coordinates

	for (int p = 0; p < m_numSets; p++)
	for (int i = 0; i < n; i++)
	for (int j = 0; j < n; j++) {
		int k = randInt(j, n - 1);
		float t = m_samples[j * n + i + p * m_numSamples][1];
		m_samples[j * n + i + p * m_numSamples][1] = m_samples[k * n + i + p * m_numSamples][1];
		m_samples[k * n + i + p * m_numSamples][1] = t;
	}
}
//////////////////////////////////////Jittered////////////////////////////////////////////////////////////
Jittered::Jittered() : Sampler(){
	generateSamples();

}

Jittered::Jittered(const int numSamples, const int numSets) : Sampler(numSamples, numSets){
	generateSamples();
}

Jittered::~Jittered() {}

void Jittered::generateSamples() {

	int n = (int)sqrt((float)m_numSamples);

	for (int p = 0; p < m_numSets; p++){
		for (int j = 0; j < n; j++){
			for (int k = 0; k < n; k++) {
				Vector2f sp((k + randFloat()) / n, (j + randFloat()) / n);
				m_samples.push_back(sp);
			}
		}
	}
}
//////////////////////////////////////Hammersley////////////////////////////////////////////////////////////
Hammersley::Hammersley() : Sampler(){
	generateSamples();

}

Hammersley::Hammersley(const int numSamples, const int numSets) : Sampler(numSamples, numSets){
	generateSamples();
}

Hammersley::~Hammersley() {}

double Hammersley::phi(int j) {
	double x = 0.0;
	double f = 0.5;

	while (j) {
		x += f * (double)(j % 2);
		j /= 2;
		f *= 0.5;
	}
	return (x);
}


void Hammersley::generateSamples() {

	for (int p = 0; p < m_numSets; p++){
		for (int j = 0; j < m_numSamples; j++) {
			Vector2f pv((float)j / (float)m_numSamples, phi(j));
			m_samples.push_back(pv);
		}
	}
}