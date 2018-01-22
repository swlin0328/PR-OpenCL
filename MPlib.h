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

#include <omp.h>

using namespace std;
using namespace cv;

namespace MPlib
{
	unsigned char to_uchar(string data);

	vector<string> string_partition(const string &source, char delim);

	void data_Pruning(string source, string dest, int dataSize, int start_Idx = 0);
}