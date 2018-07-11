#include "stdafx.h"
#include "Perceptual_hash.h"

#include "config.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <atlimage.h>
#include <windows.h>

#include <iostream>

using namespace cv;
using namespace std;

static LPCTSTR programTitle = _T("QQ游戏 - 连连看角色版");

// 使用 感知哈希算法寻找相同的方块
//////////////////////////////////////////////////////////////////////////
Vec3b calAvgColor(Mat & img)
{
	if (img.rows * img.cols == 0)
		return 0;

	int a, b, c;
	a = b = c = 0;

	for (int row = 0; row < img.rows; row++)
	{
		for (int col = 0; col < img.cols; col++)
		{
			a += img.at<Vec3b>(row, col)[0];
			b += img.at<Vec3b>(row, col)[1];
			c += img.at<Vec3b>(row, col)[2];
		}
	}
	Vec3b avg{ 0,0,0 };
	avg[0] = a / (img.rows * img.cols);
	avg[1] = b / (img.rows * img.cols);
	avg[2] = c / (img.rows * img.cols);

	return avg;
}

bool isSimilarColor(Vec3b c1, Vec3b c2)
{
	const static int diff = 30;
	int dx = c1[0] - c2[0];
	int dy = c1[1] - c2[1];
	int dz = c1[2] - c2[2];

	if (abs(dx) <= diff \
		&& abs(dy) <= diff \
		&& abs(dz) <= diff)
	{
		return true;
	}
	return false;
}

int calAvg(Mat & img)
{
	if (img.rows * img.cols == 0)
		return 0;

	int avg = 0;
	for (int row = 0; row < img.rows; row++)
	{
		for (int col = 0; col < img.cols; col++)
		{
			avg += img.at<uchar>(row, col);
		}
	}
	return avg / (img.rows * img.cols);
}

uint64 calHash(Mat & img, int avg)
{
	uint64 ret = 0;
	for (int row = 0; row < img.rows; row++)
	{
		for (int col = 0; col < img.cols; col++)
		{
			if (img.at<uchar>(row, col) >= avg)
			{
				ret |= 1i64 << (row * 8 + col);
			}
		}
	}
	return ret;
}

int haming(uint64 d1, uint64 d2)
{
	int h = 0;
	uint64 d = d1 ^ d2;

	while (d) {
		h++;
		d &= d - 1;
	}

	return h;
}

void CImageToMat(CImage& cimage, Mat& mat)
{
	if (cimage.IsNull())
	{
		return;
	}


	int nChannels = cimage.GetBPP() / 8;
	if ((1 != nChannels) && (3 != nChannels))
	{
		return;
	}
	int nWidth = cimage.GetWidth();
	int nHeight = cimage.GetHeight();


	//重建mat  
	if (1 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC1);
	}
	else if (3 == nChannels)
	{
		mat.create(nHeight, nWidth, CV_8UC3);
	}

	//拷贝数据  
	uchar* pucRow;                                  //指向数据区的行指针  
	uchar* pucImage = (uchar*)cimage.GetBits();     //指向数据区的指针  
	int nStep = cimage.GetPitch();                  //每行的字节数,注意这个返回值有正有负  


	for (int nRow = 0; nRow < nHeight; nRow++)
	{
		pucRow = (mat.ptr<uchar>(nRow));
		for (int nCol = 0; nCol < nWidth; nCol++)
		{
			if (1 == nChannels)
			{
				pucRow[nCol] = *(pucImage + nRow * nStep + nCol);
			}
			else if (3 == nChannels)
			{
				for (int nCha = 0; nCha < 3; nCha++)
				{
					pucRow[nCol * nChannels + nCha] = *(pucImage + nRow * nStep + nCol * nChannels + nCha);
				}
			}
		}
	}
}

bool calGameBoard(Mat &img, std::vector<std::vector<int>> &gameBoard)
{
	if (img.empty()) {
		return false;
	}

	vector<std::pair<uint64, Vec3b> > containerHashCode;
	containerHashCode.push_back(make_pair(0xffffffffffffffff, Vec3b{112, 76, 48})); // 空白格的颜色很特殊 // bgr

													 // 初始化
	gameBoard = std::vector<std::vector<int> >(Rows, std::vector<int>(Colunms, 0));

	for (int i = 0; i < Rows; i++)
	{
		for (int j = 0; j < Colunms; j++)
		{
			// 1. 缩放到 8 * 8 大小
			Mat s;
			resize(img(Rect{ StartPosX + (ItemWidth * j), StartPosY + (ItemHeight * i) + 4, ItemWidth - 4, ItemHeight - 4 }),
				s,
				Size{ 8, 8 });
			// 2.1 提取平均颜色值 (TODO: 提取图片的主要颜色算法需要研究一下)
			Vec3b avgColor = calAvgColor(s);
			// 2. 处理成灰度
			Mat gray_s;
			cvtColor(s, gray_s, CV_RGB2GRAY);
			// 3. 统计灰度平均值
			int avg = calAvg(gray_s);
			// 4. 遍历产生一个特征值
			uint64 hashCode = calHash(gray_s, avg);

			//cout <<"i = "<< i << " j = " << j << " = " << avgColor << endl;
			auto pos = std::find_if(begin(containerHashCode), end(containerHashCode), [hashCode, avgColor](std::pair<uint64, Vec3b> & value) {
				if (haming(hashCode, value.first) < 5) { // 形状相似
					if (isSimilarColor(avgColor, value.second)) { // 颜色相同
						return true;
					}
				}
				return false;
			});

			if (pos == end(containerHashCode)) {
				//char buf[1024] = { 0 };
				//sprintf_s(buf, "%llx-%02d-%02d.bmp", hashCode, i, j);

				//imwrite(buf, gray_s);
				containerHashCode.push_back(make_pair(hashCode, avgColor));
				gameBoard[i][j] = containerHashCode.size() - 1;
			}
			else {
				gameBoard[i][j] = std::distance(begin(containerHashCode), pos);
			}
		}
	}

	return true;
}

bool PerceptualHash::getGameBoard(std::vector<std::vector<int>> &gameBoard)
{
	HWND hwnd = FindWindow(NULL, programTitle);
	if (NULL == hwnd) {
		_tprintf(_T("FindWindow failed, error code %d"), GetLastError());
		return false;
	}

	RECT rect;
	GetWindowRect(hwnd, &rect);
	int w = rect.right - rect.left, h = rect.bottom - rect.top;
	_tprintf(_T("width = %d, height = %d\n"), w, h);

	// 获取窗口的设备上下文（Device Contexts）
	HDC hdcWindow = GetDC(hwnd); // 要截图的窗口句柄，为空则全屏
								 // 获取设备相关信息的尺寸大小
								 //int nBitPerPixel = GetDeviceCaps(hdcWindow, BITSPIXEL);
	int nBitPerPixel = 24; // 3 通道颜色
						   // 将游戏窗口截图
	CImage image;
	image.Create(w, h, nBitPerPixel);
	BitBlt(
		image.GetDC(),			// 保存到的目标 图片对象 上下文
		0, 0,					// 起始 x, y 坐标
		w, h,					// 截图宽高
		hdcWindow,				// 截取对象的 上下文句柄
		0, 0,					// 指定源矩形区域左上角的 X, y 逻辑坐标
		SRCCOPY);
	image.ReleaseDC();

	// convert to Mat struct
	Mat img;
	CImageToMat(image, img);
	imwrite("./ScreenShot.bmp", img);

	// 获取游戏棋盘
	return calGameBoard(img, gameBoard);
}