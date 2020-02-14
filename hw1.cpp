/**
 * Based on idea from Matthew Flatt, Univ of Utah
 */
#include <iostream>
#include "ThreadGroup.h"
using namespace std;



int encode(int v) {
	// do something time-consuming (and arbitrary)
	for (int i = 0; i < 500; i++)
		v = ((v * v) + v) % 10;
	return v;
}

int decode(int v) {
	// do something time-consuming (and arbitrary)
	return encode(v);
}

/**
* SharedData holds the array of integers and the size of the array
**/
struct SharedData
{
	int* data;
	int length;
};

class encodeSpeed
{
	public:
		void operator()(int id, void* sharedData)
		{
			SharedData* ourData = (SharedData*)sharedData;
			int size = ourData->length/2;
			int start = size * id;
			for (int i = start; i < start + size; i++) {
				ourData->data[i] = encode(ourData->data[i]);
			}
		}
};

class decodeSpeed
{
	public:
		void operator()(int id, void* sharedData)
		{
			SharedData* ourData = (SharedData*)sharedData;
			int size = ourData->length/2;
			int start = size * id;
			for (int i = start; i < start + size; i++) {
				ourData->data[i] = decode(ourData->data[i]);
			}
		}
};

/**
* prefixSums encodes the data in 2 threads, then sums the encoded values
* in the main thread before decoding the data in two threads
**/
void prefixSums(int *data, int length) {

	int encodedSum = 0;
	SharedData ourData;
	ourData.data = data;
	ourData.length = length;
	ThreadGroup<encodeSpeed> encodeMaster;
	encodeMaster.createThread(0, &ourData);
	encodeMaster.createThread(1, &ourData);
	encodeMaster.waitForAll();
	for (int i = 0; i < length; i++) {
		encodedSum += ourData.data[i];
		ourData.data[i] = encodedSum;
	}
	ThreadGroup<decodeSpeed> decodeMaster;
	decodeMaster.createThread(0, &ourData);
	decodeMaster.createThread(1, &ourData);
	decodeMaster.waitForAll();

}

int main() {
	int length = 1000 * 1000;

	// make array
	int *data = new int[length];
	for (int i = 1; i < length; i++)
		data[i] = 1;
	data[0] = 6;

	// transform array into converted/deconverted prefix sum of original
	prefixSums(data, length);

	// printed out result is 6, 6, and 2 when data[0] is 6 to start and the rest 1
	cout << "[0]: " << data[0] << endl
			<< "[" << length/2 << "]: " << data[length/2] << endl 
			<< "[end]: " << data[length-1] << endl; 

    delete[] data;
	return 0;
}
