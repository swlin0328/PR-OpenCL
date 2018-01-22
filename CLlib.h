#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "opencv2\opencv.hpp"
#include "opencv2\core.hpp"
#include "opencv2\imgproc.hpp"
#include "opencv2\highgui.hpp"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

#define __CLlib

using namespace std;
using namespace cv;

namespace CLlib
{
	void queueDevice();

	cl_int makeCL_Program(int deviceNum, vector<float>& source1, vector<float>& source2, vector<float>& res, const char* filename);

	vector<cl_platform_id> getPlatform();

	vector<cl_device_id> GetContextInfo(cl_context& context);

	void setMemContext(cl_context& context, cl_command_queue queue, vector<cl_mem>& mem_set, vector<float>& source1, vector<float>& source2, vector<float>& res);

	cl_program load_program(cl_context& context, const char* filename);

	void test_CL_program(int deviceNum);
}