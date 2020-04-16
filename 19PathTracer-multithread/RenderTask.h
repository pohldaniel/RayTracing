#ifndef _RENDERTASK_H
#define _RENDERTASK_H

#include <atomic>
#include <vector>
#include <thread>
#include <random>
#include <chrono>
#include <future>

#include <iostream>
#include <future>
#include <mutex>
#include <cmath>

#include "STimer.h"
#include "Camera.h"
#include "Vector.h"
#include "Color.h"



class Stoppable2 {

	std::promise<void> exitSignal;
	std::future<void> futureObj;

public:
	Stoppable2() : futureObj(exitSignal.get_future()) {

	}

	Stoppable2(Stoppable2 && obj) : exitSignal(std::move(obj.exitSignal)), futureObj(std::move(obj.futureObj)) {

		std::cout << "Move Constructor is called" << std::endl;
	}

	Stoppable2 & operator=(Stoppable2 && obj) {
		std::cout << "Move Assignment is called" << std::endl;
		exitSignal = std::move(obj.exitSignal);
		futureObj = std::move(obj.futureObj);
		return *this;
	}

	// Task need to provide defination  for this function
	// It will be called by thread function
	virtual void run(STimer& timer, unsigned char*&data, int numThread) = 0;

	// Thread function to be executed by thread
	void operator()(STimer& timer, unsigned char*&data, int numThread) {
		run(timer, data, numThread);
	}

	//Checks if thread is requested to stop
	bool stopRequested() {

		// checks if value in future object is available
		if (futureObj.wait_for(std::chrono::milliseconds(0)) == std::future_status::timeout)
			return false;
		return true;
	}
	// Request the thread to stop by setting value in promise object
	void stop() {
		exitSignal.set_value();
	}

};


class RenderTask :  public Stoppable2 {

	
public:

	float RandomFloat() {

		/*
		// Xorshift random number algorithm invented by George Marsaglia
		static uint32_t rng_state = 0xf2eec0de;
		rng_state ^= (rng_state << 13);
		rng_state ^= (rng_state >> 17);
		rng_state ^= (rng_state << 5);
		return float(rng_state) * (1.0f / 4294967296.0f);
		*/

		// alternately, using a standard c++ prng
		static std::random_device rd;
		static std::mt19937 mt(rd());
		static std::uniform_real_distribution<float> dist(0.0f, 1.0f);
		return dist(mt);
	}


	RenderTask::RenderTask() {

		

	}


	

	

	

	


	


	std::mutex mutex2;
	bool waitAllThreads2;
	bool finishedAllThreads2;
	const size_t c_imageWidth2 = 512;
	const size_t c_imageHeight2 = 512;
	const size_t c_samplesPerPixel2 = 100;



	int currentRowIndex = 0;
	bool m_wait = false;
	bool m_finished = false;

	Projection *camera;

	int getCurrentRowIndex() { return currentRowIndex; }
	void reset() { currentRowIndex = 0; m_finished = false;  m_wait = false; }
	void setWaiting() { m_wait = true; }
	void setFinished() { m_finished = true; }
	bool isFinished() { return m_finished; }




	


	// Function to be executed by thread function
	void RenderTask::run(STimer& timer, unsigned char* &pixels2, int threadnum) {

		// Check if thread is requested to stop ?
		while (stopRequested() == false) {

			mutex2.lock();
			RenderTask::waitAllThreads2 = m_wait;
			mutex2.unlock();


			// each thread grabs a pixel at a time and renders it
			//currentRowIndex = min(currentRowIndex, c_imageHeight);
			size_t rowIndex = currentRowIndex++;

			bool firstThread = rowIndex == 0;
			int lastPercent = -1;

			while (((rowIndex < c_imageHeight2 && !m_finished && !m_wait))) {

				for (size_t x = 0; x < c_imageWidth2; ++x) {

					if (m_wait) {
						break;
					}

					// render the pixel by taking multiple samples and incrementally averaging them
					for (size_t i = 0; i < c_samplesPerPixel2; ++i) {
						float jitterX = 1 ? RandomFloat() : 0.5f;
						float jitterY = 1 ? RandomFloat() : 0.5f;
						float u = ((float)x + jitterX) / (float)c_imageWidth2;
						float v = ((float)rowIndex + jitterY) / (float)c_imageHeight2;


						

						//TPixelRGBF32 sample;
						//RenderPixel(u, v, sample);
						//pixels[rowIndex * c_imageWidth + x] += (sample - pixels[rowIndex * c_imageWidth + x]) / float(i + 1.0f);
					}

					for (size_t j = 0; j < 3; j++) {

						//pixels2[rowIndex * c_imageWidth * 3 + k + j] = uint8(Clamp(powf(pixels[rowIndex * c_imageWidth + x][2 - j], 1.0f / 2.2f)* 255.0f, 0.0f, 255.0f));
					}

				}

				// move to next row
				rowIndex = currentRowIndex++;


			}// rendering



			mutex2.lock();
			finishedAllThreads2 = (currentRowIndex - 1) == c_imageHeight2;
			mutex2.unlock();
		}// stop requested

		 //std::cout << "Task End" << std::endl;
	}// end run

};
#endif // _RENDERTASK_H