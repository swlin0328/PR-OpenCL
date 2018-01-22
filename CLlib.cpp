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

	cl_int makeCL_Program(int deviceNum, vector<float>& source1, vector<float>& source2, vector<float>& res, const char* filename)
	{
		cl_int err;
		vector<cl_platform_id> platforms{ getPlatform() };

		//create context
		cl_context_properties prop[] = { CL_CONTEXT_PLATFORM, reinterpret_cast<cl_context_properties>(platforms[deviceNum]), 0 };
		cl_context context = clCreateContextFromType(prop, CL_DEVICE_TYPE_DEFAULT, NULL, NULL, NULL);
		if (context == 0)
		{
			std::cerr << "Can't create OpenCL context\n";
		}
		vector<cl_device_id> devices{ GetContextInfo(context) };

		//command_queue
		cl_command_queue queue = clCreateCommandQueue(context, devices[0], 0, 0);
		if (queue == 0)
		{
			std::cerr << "Can't create command queue\n";
			clReleaseContext(context);
		}

		//set memory and data on CL device
		vector<cl_mem> mem_set;
		setMemContext(context, queue, mem_set, source1, source2, res);

		//build program on CL device
		cl_program program = load_program(context, filename);
		if (program == 0)
		{
			std::cerr << "Can't load or build program\n";
			clReleaseMemObject(mem_set[0]);
			clReleaseMemObject(mem_set[1]);
			clReleaseMemObject(mem_set[2]);
			clReleaseCommandQueue(queue);
			clReleaseContext(context);
		}

		//load program to kernel
		cl_kernel kernel_Cal = clCreateKernel(program, "kernel_Cal", 0);
		if (kernel_Cal == 0)
		{
			std::cerr << "Can't load kernel\n";
			clReleaseProgram(program);
			clReleaseMemObject(mem_set[0]);
			clReleaseMemObject(mem_set[1]);
			clReleaseMemObject(mem_set[2]);
			clReleaseCommandQueue(queue);
			clReleaseContext(context);
		}

		//Set program arguments and read the results to main
		clSetKernelArg(kernel_Cal, 0, sizeof(cl_mem), &mem_set[0]);
		clSetKernelArg(kernel_Cal, 1, sizeof(cl_mem), &mem_set[1]);
		clSetKernelArg(kernel_Cal, 2, sizeof(cl_mem), &mem_set[2]);

		size_t work_size = source1.size();
		err = clEnqueueNDRangeKernel(queue, kernel_Cal, 1, 0, &work_size, 0, 0, 0, 0);
		if (err == CL_SUCCESS)
		{
			err = clEnqueueReadBuffer(queue, mem_set[2], CL_TRUE, 0, sizeof(float) * res.size(), &res[0], 0, 0, 0);
		}

		clReleaseKernel(kernel_Cal);
		clReleaseProgram(program);
		clReleaseMemObject(mem_set[0]);
		clReleaseMemObject(mem_set[1]);
		clReleaseMemObject(mem_set[2]);
		clReleaseCommandQueue(queue);
		clReleaseContext(context);
		return err;
	}

	vector<cl_platform_id> getPlatform()
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
		return platforms;
	}

	vector<cl_device_id> GetContextInfo(cl_context& context)
	{
		size_t cb;
		clGetContextInfo(context, CL_CONTEXT_DEVICES, 0, NULL, &cb);
		std::vector<cl_device_id> devices(cb / sizeof(cl_device_id));
		clGetContextInfo(context, CL_CONTEXT_DEVICES, cb, &devices[0], 0);

		clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 0, NULL, &cb);
		std::string devname;
		devname.resize(cb);
		clGetDeviceInfo(devices[0], CL_DEVICE_NAME, cb, &devname[0], 0);
		std::cout << "Device: " << devname.c_str() << "\n";

		return devices;
	}

	void setMemContext(cl_context& context, cl_command_queue queue, vector<cl_mem>& mem_set, vector<float>& source1, vector<float>& source2, vector<float>& res)
	{
		//Create memory on CL device
		cl_mem cl_a = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * source1.size(), &source1[0], NULL);
		cl_mem cl_b = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(cl_float) * source2.size(), &source2[0], NULL);
		cl_mem cl_res = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(cl_float) * res.size(), NULL, NULL);

		mem_set.push_back(cl_a);
		mem_set.push_back(cl_b);
		mem_set.push_back(cl_res);

		if (cl_a == 0 || cl_b == 0 || cl_res == 0)
		{
			std::cerr << "Can't create OpenCL buffer\n";
			clReleaseMemObject(cl_a);
			clReleaseMemObject(cl_b);
			clReleaseMemObject(cl_res);
			clReleaseCommandQueue(queue);
			clReleaseContext(context);
		}
	}

	cl_program load_program(cl_context& context, const char* filename)
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

	void test_CL_program(int deviceNum)
	{
		cl_int err;

		const int DATA_SIZE = 1048576;
		std::vector<float> test_data1(DATA_SIZE), test_data2(DATA_SIZE), res(DATA_SIZE);
		for (int i = 0; i < DATA_SIZE; i++)
		{
			test_data1[i] = std::rand();
			test_data2[i] = std::rand();
		}
		err = makeCL_Program(deviceNum, test_data1, test_data2, res, "testCL.cl");

		if (err == CL_SUCCESS)
		{
			bool correct = true;
			for (int i = 0; i < DATA_SIZE; i++) {
				if (test_data1[i] + test_data2[i] != res[i])
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
	}
}