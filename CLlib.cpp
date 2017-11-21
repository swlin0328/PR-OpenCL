#include "CLlib.h"

namespace CLlib
{
	void queueDevice()
	{
		cl_int err;
		cl_uint num;
		err = clGetPlatformIDs(0, 0, &num);
		if (err != CL_SUCCESS) 
		{
			std::cerr << "Unable to get platforms\n";
		}

		std::vector<cl_platform_id> platforms(num);
		err = clGetPlatformIDs(num, &platforms[0], &num);
		if (err != CL_SUCCESS) 
		{
			std::cerr << "Unable to get platform ID\n";
		}

		for (cl_uint i = 0; i < num; i++)
		{
			cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platforms[i]), 0 };
			cl_context context = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, NULL);
			if (context == 0 || context == NULL)
			{
				std::cerr << "Can't create OpenCL context\n";
			}

			size_t cb;
			clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
			std::vector<cl_device_id> devices(cb / sizeof(cl_device_id));
			clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, &devices[0], 0);

			clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &cb);
			std::string devname;
			devname.resize(cb);
			clGetDeviceInfo(devices[0], CL_DEVICE_NAME, cb, &devname[0], 0);
			std::cout << "========== Device " << i << " ==========\n" << "Plateform: " << devname.c_str() << "\n\n";
			clReleaseContext(context);
		}
	}

	void testCL(int deviceNum)
	{
		cl_int err;
		cl_uint num;
		err = clGetPlatformIDs(0, 0, &num);
		if (err != CL_SUCCESS) 
		{
			std::cerr << "Unable to get platforms\n";
		}

		std::vector<cl_platform_id> platforms(num);
		err = clGetPlatformIDs(num, &platforms[0], &num);
		if (err != CL_SUCCESS) 
		{
			std::cerr << "Unable to get platform ID\n";
		}

		cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platforms[deviceNum]), 0 };
		cl_context context = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, NULL);
		if (context == 0) 
		{
			std::cerr << "Can't create OpenCL context\n";
		}

		size_t cb;
		clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
		std::vector<cl_device_id> devices(cb / sizeof(cl_device_id));
		clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, &devices[0], 0);

		clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &cb);
		std::string devname;
		devname.resize(cb);
		clGetDeviceInfo(devices[0], CL_DEVICE_NAME, cb, &devname[0], 0);
		std::cout << "Device: " << devname.c_str() << "\n";

		cl_command_queue queue = clCreateCommandQueue(context, devices[0], 0, 0);
		if (queue == 0) 
		{
			std::cerr << "Can't create command queue\n";
			clReleaseContext(context);
		}

		const int DATA_SIZE = 1048576;
		std::vector<float> a(DATA_SIZE), b(DATA_SIZE), res(DATA_SIZE);
		for (int i = 0; i < DATA_SIZE; i++) 
		{
			a[i] = std::rand();
			b[i] = std::rand();
		}

		cl_mem cl_a = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * DATA_SIZE, &a[0], NULL);
		cl_mem cl_b = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * DATA_SIZE, &b[0], NULL);
		cl_mem cl_res = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * DATA_SIZE, NULL, NULL);
		if (cl_a == 0 || cl_b == 0 || cl_res == 0) 
		{
			std::cerr << "Can't create OpenCL buffer\n";
			clReleaseMemObject(cl_a);
			clReleaseMemObject(cl_b);
			clReleaseMemObject(cl_res);
			clReleaseCommandQueue(queue);
			clReleaseContext(context);
		}

		cl_program program = load_program(context, "testCL.cl");
		if (program == 0) 
		{
			std::cerr << "Can't load or build program\n";
			clReleaseMemObject(cl_a);
			clReleaseMemObject(cl_b);
			clReleaseMemObject(cl_res);
			clReleaseCommandQueue(queue);
			clReleaseContext(context);
		}

		cl_kernel adder = clCreateKernel(program, "adder", 0);
		if (adder == 0) 
		{
			std::cerr << "Can't load kernel\n";
			clReleaseProgram(program);
			clReleaseMemObject(cl_a);
			clReleaseMemObject(cl_b);
			clReleaseMemObject(cl_res);
			clReleaseCommandQueue(queue);
			clReleaseContext(context);
		}

		clSetKernelArg(adder, 0, sizeof(cl_mem), &cl_a);
		clSetKernelArg(adder, 1, sizeof(cl_mem), &cl_b);
		clSetKernelArg(adder, 2, sizeof(cl_mem), &cl_res);

		size_t work_size = DATA_SIZE;
		err = clEnqueueNDRangeKernel(queue, adder, 1, 0, &work_size, 0, 0, 0, 0);

		if (err == CL_SUCCESS) 
		{
			err = clEnqueueReadBuffer(queue, cl_res, CL_TRUE, 0, sizeof(float) * DATA_SIZE, &res[0], 0, 0, 0);
		}

		if (err == CL_SUCCESS) 
		{
			bool correct = true;
			for (int i = 0; i < DATA_SIZE; i++) {
				if (a[i] + b[i] != res[i]) 
				{
					correct = false;
					break;
				}
			}

			if (correct) 
			{
				std::cout << "Data is correct\n";
			}
			else 
			{
				std::cout << "Data is incorrect\n";
			}
		}
		else 
		{
			std::cerr << "Can't run kernel or read back data\n";
		}

		clReleaseKernel(adder);
		clReleaseProgram(program);
		clReleaseMemObject(cl_a);
		clReleaseMemObject(cl_b);
		clReleaseMemObject(cl_res);
		clReleaseCommandQueue(queue);
		clReleaseContext(context);
	}


	cl_program load_program(cl_context context, const char* filename)
	{
		std::ifstream in(filename, std::ios_base::binary);
		if (!in.good()) 
		{
			return 0;
		}

		// get file length
		in.seekg(0, std::ios_base::end);
		size_t length = in.tellg();
		in.seekg(0, std::ios_base::beg);

		// read program source
		std::vector<char> data(length + 1);
		in.read(&data[0], length);
		data[length] = 0;

		// create and build program 
		const char* source = &data[0];
		cl_program program = clCreateProgramWithSource(context, 1, &source, 0, 0);
		if (program == 0) 
		{
			return 0;
		}

		if (clBuildProgram(program, 0, 0, 0, 0, 0) != CL_SUCCESS) 
		{
			return 0;
		}

		return program;
	}
}