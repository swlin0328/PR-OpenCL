#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

using namespace std;

namespace CLlib
{
	void queueDevice();

	void testCL(int deviceNum);

	cl_program load_program(cl_context context, const char* filename);

	unsigned char to_uchar(string data);

	void data_Pruning(string source, string dest, int start_Idx = 0);
}