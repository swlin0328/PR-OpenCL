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

#include <omp.h>

int main()
{

	#pragma omp parallel
	{
		printf("<T:%d> - %d \n", omp_get_thread_num(), 0);
	}

	system("pause");
	return 0;
}