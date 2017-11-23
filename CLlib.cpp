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

	void data_Pruning(string source, string dest, int dataSize, int start_Idx)
	{
		int num_thread = omp_get_num_threads() - 1;
		ifstream iData(source, ios::in);
		vector<stringstream> oData(num_thread);
		vector<string> raw_Data;
		string head, line;
		getline(iData, head);

		//變數宣告
		vector<Mat> srcImages;
		vector<vector<uchar>> srcVec;

		for (int i = 0; i < dataSize; i++)
		{
			srcImages.push_back(Mat::zeros(Size(28, 28), CV_8UC1));
			srcVec.push_back(vector<uchar>(28 * 28, 0));
		}

		#pragma omp parallel num_threads(num_thread) shared(start_Idx, raw_Data, srcImages, srcVec, oData)
		{
			//讀取圖片
			#pragma omp master
			{
				int idx = 0;
				while (iData.peek() != EOF && getline(iData, line) && idx < dataSize)
				{
					idx++;
					#pragma omp task shared(srcImages) firstprivate(idx)
					{
						vector<uchar> img;
						vector<string> readData;
						readData = string_partition(line, ',');

						for (int i = start_Idx; i < readData.size(); i++)
						{
							img.push_back(to_uchar(readData[i]));
						}
						Mat img_Mat(Size(28, 28), CV_8UC1, img.data());
						cv::swap(srcImages[idx], img_Mat);
					}
				}
				#pragma omp taskwait
				iData.close();
			}

			//處理雜訊
			#pragma omp for nowait schedule(static)
			for (int i = 0; i < srcImages.size(); i++)
			{
				Mat dst1, dst2;

				//blur
				medianBlur(srcImages[i], dst1, 3);
				cv::swap(srcImages[i], dst1);

				//sharp
				equalizeHist(srcImages[i], dst2);
				cv::swap(srcImages[i], dst2);

				//Mat to 1-dim
				srcImages[i] = srcImages[i].reshape(0, 1);
				const uchar* p = srcImages[i].data;
				vector<uchar> src(p, p + srcImages[i].cols);
				std::swap(srcVec[i], src);
			}

			//輸出圖片
			int thread_Id = omp_get_thread_num();

			#pragma omp for schedule(static)
			for (int i = 0; i < srcVec.size(); i++)
			{
				for (int j = 0; j < srcVec[i].size(); j++)
				{
					oData[thread_Id] << static_cast<unsigned>(srcVec[i][j]) << ",";
				}
				oData[thread_Id] << endl;
			}
			#pragma omp barrier

			//輸出結果
			#pragma omp single
			{
				ofstream oImg(dest, ios::out);
				oImg << head << endl;
				for (int i = 0; i < num_thread; i++)
				{
					while (oData[i].peek() != EOF && getline(oData[i], line))
					{
						string temp_Img;
						oData[i] >> temp_Img;
						oImg << temp_Img << endl;
					}
				}
				oImg.close();
			}
		}
	}

	unsigned char to_uchar(string data)
	{
		istringstream iData(data);
		unsigned char val;
		iData >> val;

		return val;
	}

	vector<string> string_partition(const string &source, char delim)
	{
		vector<string> result;
		stringstream ss;
		ss.str(source);
		string item;

		while (getline(ss, item, delim))
		{
			result.push_back(item);
		}
		return result;
	}

	string to_word(int Val)
	{
		char tempWord[20];
		sprintf_s(tempWord, 20, "%d", Val);

		return string{ tempWord };
	}
}