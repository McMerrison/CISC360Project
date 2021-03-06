/*
* Modified by: Steven Sell
* Optimized simulation of Painter's Algorithm
*
* Creates an array of pixels defined by a Color and Depth
* Uses a nested for loop to set the values for a select set of pixels
* Starting with the least depth (closest to zbuffer), draw pixels from front to back
* Pixel color (int value) and depth (floating point) are generated randomly
* Update pixel only if it has not been previously defined, then update depth
* 
* Base Optimizations:
* This implementation was developed to further optimize our Painter's Algorithm
* approach. We changed variables to constants, have less function calls (and less
* passing around of data), created an array of random numbers to call instead of 
* calling a function with rand() every iteration, and we use 2D arrays in a more 
* efficient manner. With these base optimizations we generate a sequential run
* configuration.
*
* Parallel Optimizations:
* We also utilize parallel programming (OpenACC and OpenMP) to generate two additional
* run configurations for testing speed. OpenACC uses cores of the GPU while OpenMP uses
* CPU cores to run the following code. You can see this by the #pragma declarations for
* each type.
*
* In total, we compile three run configurations: Optimized Sequential, OpenMP, and OpenACC.
*/
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <array>
#include <chrono>
#pragma warning (disable : 3180 )
void printBuffer(unsigned int *color, int width) {
	for (int i = 0; i < width; i ++) {
		for (int j = 0; j < width; j++) {
			printf("%d", color[i*width+j]);
		}
	}
}

const unsigned int ITERS = 7; //Number Of Iterations (with increasing array sizes each)
const unsigned int FPS = 100; //Number Of Frame Updates For The zbuffer
const unsigned int DIM = 100; //Initial Dimension For Array Size
const unsigned int MAX = DIM * (1 << ITERS); //Max Array Size Based On Dimension And Number Of Iterations
float timeSave;

int main(int argc, char* argv[]) {
	if(argc > 1) {
		if(atoi(argv[1]) == 1) {
		printf("Frame Buffer: %d \n\n",FPS);
		
		printf("ACC Implementation \n");
		printf("+----------------+---------+ \n");
		printf("|   Array Size   | Time(s) | \n");
		}
	} else {
		printf("+----------------+---------+ \n");
		printf("|   Array Size   | Time(s) | \n");
	}
	
	srand(time(NULL));
	
	auto randArrC = new int[MAX][MAX];
	auto randArrD = new float[MAX][MAX];
	
	//Generate Random Arrays for Color and Depth
	for(int j = 0;j < MAX; j++) {
		for(int k = 0;k < MAX; k++) {
			randArrC[j][k] = (rand() % 10);
			randArrD[j][k] = (rand() % 10)/10.0f;
		}
	}
	
	#pragma acc data copyin(randArrC[0:MAX][0:MAX],randArrD[0:MAX][0:MAX])
	{
	//Loop based on Dimension->Max *2
	for (unsigned int w = DIM; w <= MAX; w *= 2) {		
		
		unsigned int * color = new unsigned int[w*w];
		float * depth = new float[w*w];
		
		for (unsigned int i = 0; i < w; i ++) {
			for (unsigned int j = 0; j < w; j++) {
				unsigned int count = i*w+j;
				color[count] = 0;
				depth[count] = 1.0f;
			}
		}
		
		//Initialize time tracking
		std::chrono::time_point <std::chrono::steady_clock> begin, end;
		
		//Pass Color & Depth CPU->GPU (to be passed back to CPU after)
		#pragma acc data copy(color[0:w*w],depth[0:w*w])
		{
		//Start tracking time
		begin = std::chrono::steady_clock::now();
		//Iterate through frames
		for (unsigned int b = 0; b < FPS; ++b) {
			//Pass current frame CPU->GPU and begin looping through array of Colors/Depths
			#pragma omp parallel for
			#pragma acc kernels loop independent copyin(b)
			for (unsigned int i = 0; i < w; ++i) {
				#pragma omp simd
				#pragma acc loop independent
				for (unsigned int j = 0; j < w; j+=2) {
					//If the random number pulled is less than the depth ...
					if (randArrD[i][(j+b)%MAX] < depth[i*w+j]) {
						//Update the Pixel with the new random color and depth
						color[i*w+j] = randArrC[i][(j+b)%MAX];
						depth[i*w+j] = randArrD[i][(j+b)%MAX];
					}
					if (randArrD[i][(j+1+b)%MAX] < depth[i*w+j+1]) {
						color[i*w+j+1] = randArrC[i][(j+1+b)%MAX];
						depth[i*w+j+1] = randArrD[i][(j+1+b)%MAX];
					}
				}
			}
		}
		//Stop tracking time
		end = std::chrono::steady_clock::now();
		}
		
		//After, should be array of random numbers (each represents pixel color)
		if (ITERS < 3) {
			printBuffer(color, w); 
		}
		
		//Get the time taken to run
		double diff = std::chrono::duration <double> { end - begin }.count();
		
		if(w < 1000) {
			printf("+----------------+---------+ \n");
			printf("|   %d  X   %d | %.5f | \n", w, w, diff);
		} else if(w < 10000) {
			printf("+----------------+---------+ \n");
			printf("|  %d  X  %d | %.5f | \n", w, w, diff);
		} else {
			printf("+----------------+---------+ \n");
			printf("| %d  X %d | %.5f | \n", w, w, diff);
		}
		
		delete[] color;
		delete[] depth;
	}
	}
	printf("+----------------+---------+ \n");
	delete[] randArrC;
	delete[] randArrD;
}





