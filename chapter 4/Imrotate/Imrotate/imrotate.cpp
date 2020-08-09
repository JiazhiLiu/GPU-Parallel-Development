#include<opencv2\opencv.hpp>
#include<iostream>
#include<time.h>
#include<math.h>
#include<thread>

#define REPS 10
#define MAXTHREADS 20

using namespace cv;




void rotateAt(Mat &input, Mat &output, double rotAngle)
{
	// res: 430.6ms
	Size imgSize = input.size();
	int cy = imgSize.height / 2; 
	int cx = imgSize.width / 2;
	for (int y = 0; y < imgSize.height; y++) {
		for (int x = 0; x < imgSize.width; x++) {
			int X = x - cx;
			int Y = y - cy;
			double nx = cos(rotAngle)*X - sin(rotAngle)*Y;
			double ny = sin(rotAngle)*X + cos(rotAngle)*Y;

			double diagonal = sqrt((double)imgSize.height*imgSize.height + imgSize.width*imgSize.width);
			double scaleFactor = (imgSize.width > imgSize.height) ? imgSize.height / diagonal : imgSize.width / diagonal;
			nx *= scaleFactor;
			ny *= scaleFactor;
			int nnx = nx + cx;
			int nny = ny + cy;
			if (nnx >= 0 && nnx < imgSize.width && nny >= 0 && nny < imgSize.height) {
				output.at<Vec3b>(nny, nnx) = input.at<Vec3b>(y, x);
			}
		}
	}
}

void rotatePtr(Mat &input, Mat &output, double rotAngle)
{
	// res:186.1ms
	Size imgSize = input.size();
	int cy = imgSize.height / 2;
	int cx = imgSize.width / 2;
	for (int y = 0; y < imgSize.height; y++) {
		uchar *inPr = input.ptr<uchar>(y);
		for (int x = 0; x < imgSize.width; x++) {
			int X = x - cx;
			int Y = y - cy;
			double nx = cos(rotAngle)*X - sin(rotAngle)*Y;
			double ny = sin(rotAngle)*X + cos(rotAngle)*Y;

			double diagonal = sqrt((double)imgSize.height*imgSize.height + imgSize.width*imgSize.width);
			double scaleFactor = (imgSize.width > imgSize.height) ? imgSize.height / diagonal : imgSize.width / diagonal;
			nx *= scaleFactor;
			ny *= scaleFactor;
			int nnx = nx + cx;
			int nny = ny + cy;
			if (nnx >= 0 && nnx < imgSize.width && nny >= 0 && nny < imgSize.height) {
				// output.at<Vec3b>(nny, nnx) = input.at<Vec3b>(y, x);
				uchar *outPr = output.ptr<uchar>(nny);
				outPr[nnx * 3] = inPr[x * 3];
				outPr[nnx * 3 + 1] = inPr[x * 3 + 1];
				outPr[nnx * 3 + 2] = inPr[x * 3 + 2];
			}
		}
	}
}

void rotatePtrC(Mat &input, Mat &output, double rotAngle, int nthread, int nthreads)
{
	// 多线程版本
	// res:(nthreads, time) (1 182.6) (2 99.4) (3 75.4) (4 67.6) (5 78.4) (6 74.1)
	Size imgSize = input.size();
	int cy = imgSize.height / 2;
	int cx = imgSize.width / 2;
	int r = imgSize.height / nthreads;
	int be = (nthread - 1) * r;
	int en = nthread * r;
	for (int y = be; y < en; y++) {
		uchar *inPr = input.ptr<uchar>(y);
		for (int x = 0; x < imgSize.width; x++) {
			int X = x - cx;
			int Y = y - cy;
			double nx = cos(rotAngle)*X - sin(rotAngle)*Y;
			double ny = sin(rotAngle)*X + cos(rotAngle)*Y;

			double diagonal = sqrt((double)imgSize.height*imgSize.height + imgSize.width*imgSize.width);
			double scaleFactor = (imgSize.width > imgSize.height) ? imgSize.height / diagonal : imgSize.width / diagonal;
			nx *= scaleFactor;
			ny *= scaleFactor;
			int nnx = nx + cx;
			int nny = ny + cy;
			if (nnx >= 0 && nnx < imgSize.width && nny >= 0 && nny < imgSize.height) {
				uchar *outPr = output.ptr<uchar>(nny);
				outPr[nnx * 3] = inPr[x * 3];
				outPr[nnx * 3 + 1] = inPr[x * 3 + 1];
				outPr[nnx * 3 + 2] = inPr[x * 3 + 2];
			}
		}
	}
}

void rotatePtrC2(Mat &input, Mat &output, double rotAngle, int nthread, int nthreads)
{
	// 多线程版本, 相对于rotatePtrC1, 将diagonal和scaleFactor移到循环外面
	// res:(nthreads, time) (1 133.5) (2 76.9) (3 58.4) (4 51.6) (5 67.9) (6 62.6)
	Size imgSize = input.size();
	int cy = imgSize.height / 2;
	int cx = imgSize.width / 2;
	int r = imgSize.height / nthreads;
	int be = (nthread - 1) * r;
	int en = nthread * r;
	double diagonal = sqrt((double)imgSize.height*imgSize.height + imgSize.width*imgSize.width);
	double scaleFactor = (imgSize.width > imgSize.height) ? imgSize.height / diagonal : imgSize.width / diagonal;

	for (int y = be; y < en; y++) {
		uchar *inPr = input.ptr<uchar>(y);
		for (int x = 0; x < imgSize.width; x++) {
			int X = x - cx;
			int Y = y - cy;
			double nx = cos(rotAngle)*X - sin(rotAngle)*Y;
			double ny = sin(rotAngle)*X + cos(rotAngle)*Y;

			nx *= scaleFactor;
			ny *= scaleFactor;
			int nnx = (int)nx + cx;
			int nny = (int)ny + cy;
			if (nnx >= 0 && nnx < imgSize.width && nny >= 0 && nny < imgSize.height) {
				uchar *outPr = output.ptr<uchar>(nny);
				outPr[nnx * 3] = inPr[x * 3];
				outPr[nnx * 3 + 1] = inPr[x * 3 + 1];
				outPr[nnx * 3 + 2] = inPr[x * 3 + 2];
			}
		}
	}
}

void rotatePtrC3(Mat &input, Mat &output, double rotAngle, int nthread, int nthreads)
{
	// 多线程版本 cos() 和 sin() 计算移到循环外面
	// res:(nthreads, time) (1 105.7) (2 58.3) (3 42.1) (4 38.3) (5 51.5) (46.4)
	Size imgSize = input.size();
	int cy = imgSize.height / 2;
	int cx = imgSize.width / 2;
	int r = imgSize.height / nthreads;
	int be = (nthread - 1) * r;
	int en = nthread * r;
	double diagonal = sqrt((double)imgSize.height*imgSize.height + imgSize.width*imgSize.width);
	double scaleFactor = (imgSize.width > imgSize.height) ? imgSize.height / diagonal : imgSize.width / diagonal;
	double CRA = cos(rotAngle);
	double SRA = sin(rotAngle);

	for (int y = be; y < en; y++) {
		uchar *inPr = input.ptr<uchar>(y);
		for (int x = 0; x < imgSize.width; x++) {
			int X = x - cx;
			int Y = y - cy;
			double nx = CRA*X - SRA*Y;
			double ny = SRA*X + CRA*Y;

			nx *= scaleFactor;
			ny *= scaleFactor;
			int nnx = (int)nx + cx;
			int nny = (int)ny + cy;
			if (nnx >= 0 && nnx < imgSize.width && nny >= 0 && nny < imgSize.height) {
				uchar *outPr = output.ptr<uchar>(nny);
				outPr[nnx * 3] = inPr[x * 3];
				outPr[nnx * 3 + 1] = inPr[x * 3 + 1];
				outPr[nnx * 3 + 2] = inPr[x * 3 + 2];
			}
		}
	}
}

void rotatePtrC4(Mat &input, Mat &output, double rotAngle, int nthread, int nthreads)
{
	// 多线程版本 预分配内存
	// res:(nthreads, time) (1 105.5) (2 56.3) (3 42.7) (4 38.8) (5 51.9) (6 46.4) 几乎没有任何变化
	Size imgSize = input.size();
	int cy = imgSize.height / 2;
	int cx = imgSize.width / 2;
	int r = imgSize.height / nthreads;
	int be = (nthread - 1) * r;
	int en = nthread * r;
	double diagonal = sqrt((double)imgSize.height*imgSize.height + imgSize.width*imgSize.width);
	double scaleFactor = (imgSize.width > imgSize.height) ? imgSize.height / diagonal : imgSize.width / diagonal;
	double CRA = cos(rotAngle);
	double SRA = sin(rotAngle);

	int X, Y, nnx, nny;
	double nx, ny;

	for (int y = be; y < en; y++) {
		uchar *inPr = input.ptr<uchar>(y);
		for (int x = 0; x < imgSize.width; x++) {
			X = x - cx;
			Y = y - cy;
			nx = CRA*X - SRA*Y;
			ny = SRA*X + CRA*Y;

			nx *= scaleFactor;
			ny *= scaleFactor;
			nnx = (int)nx + cx;
			nny = (int)ny + cy;
			if (nnx >= 0 && nnx < imgSize.width && nny >= 0 && nny < imgSize.height) {
				uchar *outPr = output.ptr<uchar>(nny);
				outPr[nnx * 3] = inPr[x * 3];
				outPr[nnx * 3 + 1] = inPr[x * 3 + 1];
				outPr[nnx * 3 + 2] = inPr[x * 3 + 2];
			}
		}
	}
}

void rotatePtrC5(Mat &input, Mat &output, double rotAngle, int nthread, int nthreads)
{
	// 多线程版本 行内预先计算cos*y
	// res:(nthreads, time) (1 93.4) (2 51.3) (3 38.5) (4 35.5) (5 46.8) (6 43)
	Size imgSize = input.size();
	int cy = imgSize.height / 2;
	int cx = imgSize.width / 2;
	int r = imgSize.height / nthreads;
	int be = (nthread - 1) * r;
	int en = nthread * r;
	double diagonal = sqrt((double)imgSize.height*imgSize.height + imgSize.width*imgSize.width);
	double scaleFactor = (imgSize.width > imgSize.height) ? imgSize.height / diagonal : imgSize.width / diagonal;
	double CRA = cos(rotAngle);
	double SRA = sin(rotAngle);

	int X, Y, nnx, nny;
	double nx, ny;

	for (int y = be; y < en; y++) {
		uchar *inPr = input.ptr<uchar>(y);
		Y = y - cy;
		double SRAY = SRA*Y;
		double CRAY = CRA*Y;
		for (int x = 0; x < imgSize.width; x++) {
			X = x - cx;
			nx = CRA*X - SRAY;
			ny = SRA*X + CRAY;

			nx *= scaleFactor;
			ny *= scaleFactor;
			nnx = (int)nx + cx;
			nny = (int)ny + cy;
			if (nnx >= 0 && nnx < imgSize.width && nny >= 0 && nny < imgSize.height) {
				uchar *outPr = output.ptr<uchar>(nny);
				outPr[nnx * 3] = inPr[x * 3];
				outPr[nnx * 3 + 1] = inPr[x * 3 + 1];
				outPr[nnx * 3 + 2] = inPr[x * 3 + 2];
			}
		}
	}
}

void rotatePtrC6(Mat &input, Mat &output, double rotAngle, int nthread, int nthreads)
{
	// 多线程版本 最终版本
	// res:(nthreads, time) (1 65.8) (2 38.7) (3 32) (4 32) (5 38.5) (6 36.9)
	Size imgSize = input.size();
	int cy = imgSize.height / 2;
	int cx = imgSize.width / 2;
	int r = imgSize.height / nthreads;
	int be = (nthread - 1) * r;
	int en = nthread * r;
	double diagonal = sqrt((double)imgSize.height*imgSize.height + imgSize.width*imgSize.width);
	double scaleFactor = (imgSize.width > imgSize.height) ? imgSize.height / diagonal : imgSize.width / diagonal;
	double CRA = cos(rotAngle);
	double SRA = sin(rotAngle);
	CRA *= scaleFactor;
	SRA *= scaleFactor;

	int X, Y, nnx, nny;
	double nx, ny;

	for (int y = be; y < en; y++) {
		uchar *inPr = input.ptr<uchar>(y);
		Y = y - cy;
		double SRAY = SRA*Y;
		double CRAY = CRA*Y;
		double nnx0 = -cx * CRA - SRAY + cx;
		double nny0 = -cx * SRA + CRAY + cy;
		for (int x = 0; x < imgSize.width; x++) {
			nnx = (int)nnx0;
			nny = (int)nny0;
			if (nnx >= 0 && nnx < imgSize.width && nny >= 0 && nny < imgSize.height) {
				uchar *outPr = output.ptr<uchar>(nny);
				outPr[nnx * 3] = inPr[x * 3];
				outPr[nnx * 3 + 1] = inPr[x * 3 + 1];
				outPr[nnx * 3 + 2] = inPr[x * 3 + 2];
			}
			nnx0 += CRA;
			nny0 += SRA;
		}
	}
}

void rotatePtrM(Mat &input, Mat &output, double rotAngle)
{
	// res:182.9ms, 相对于rotatePtr，几乎没有改变
	Size imgSize = input.size();
	uchar Buffer[10240];
	int hbytes = imgSize.width * 3;
	int cy = imgSize.height / 2;
	int cx = imgSize.width / 2;
	for (int y = 0; y < imgSize.height; y++) {
		uchar *inPr = input.ptr<uchar>(y);
		memcpy((void*)Buffer, (void*)inPr, (size_t)hbytes);
		for (int x = 0; x < imgSize.width; x++) {
			int X = x - cx;
			int Y = y - cy;
			double nx = cos(rotAngle)*X - sin(rotAngle)*Y;
			double ny = sin(rotAngle)*X + cos(rotAngle)*Y;

			double diagonal = sqrt((double)imgSize.height*imgSize.height + imgSize.width*imgSize.width);
			double scaleFactor = (imgSize.width > imgSize.height) ? imgSize.height / diagonal : imgSize.width / diagonal;
			nx *= scaleFactor;
			ny *= scaleFactor;
			int nnx = nx + cx;
			int nny = ny + cy;
			if (nnx >= 0 && nnx < imgSize.width && nny >= 0 && nny < imgSize.height) {
				// output.at<Vec3b>(nny, nnx) = input.at<Vec3b>(y, x);
				uchar *outPr = output.ptr<uchar>(nny);
				outPr[nnx * 3] = Buffer[x * 3];
				outPr[nnx * 3 + 1] = Buffer[x * 3 + 1];
				outPr[nnx * 3 + 2] = Buffer[x * 3 + 2];
			}
		}
	}
}

int main()
{
	std::thread threads[MAXTHREADS];
	int nThreads = 3;
	Mat img = imread("./imgs/test.png");
	Mat res = Mat::zeros(img.size(), img.type());
	clock_t start, stop;
	int rotDegree = 45;
	double rotAngle = 3.141582 / 180 * rotDegree;

	start = clock();
	for (int i = 0; i < REPS; i++) {
		for (int t = 1; t <= nThreads; t++) {
			threads[t] = std::thread(rotatePtrC6, std::ref(img), std::ref(res), rotAngle, t, nThreads);
		}
		for (int t = 1; t <= nThreads; t++) {
			threads[t].join();
		}
	}
	
	stop = clock();
	
	/*imshow("a", res);
	cvWaitKey(0);*/
	std::cout << "execute time:" << 1000 * ((double)stop - start) / CLOCKS_PER_SEC / REPS << std::endl;
	system("pause");
	return 0;
}