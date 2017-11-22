#include <iostream>
#include <cstdlib>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include "CLlib.h"

#ifdef __APPLE__
#include <OpenCL/opencl.h>
#else
#include <CL/cl.h>
#endif

int main()
{
	string input_Path{ R"(C:\Users\silent-HDD\Desktop\digit\test.csv)" };
	string output_Path{ R"(C:\Users\silent-HDD\Desktop\digit\test_1.csv)" };
	CLlib::data_Pruning(input_Path, output_Path, 24000, 0);

	system("pause");
	return 0;
}