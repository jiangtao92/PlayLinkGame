// PlayLinkGame.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include "config.h"
#include "CalLinkGame.h"
#include "CMouseControl.h"
#include "Perceptual_hash.h"
#include "timer.hpp"

#include <atlimage.h>
#include <windows.h>

#include <vector>
#include <iostream>
#include <algorithm>
#include <map>
#include <cassert>
#include <string>
#include <thread>

using namespace std;

LPCTSTR programTitle = _T("QQ游戏 - 连连看角色版");

/* 获取图片的二进制数据 */
BYTE * getAddr(CImage & image)
{
	if (image.GetPitch() < 0) {
		return static_cast<BYTE *>(image.GetPixelAddress(0, image.GetHeight() - 1));
	}
	else {
		return static_cast<BYTE *>(image.GetPixelAddress(0, 0));
	}
}

/* 判断当前图片是否是空白背景 */
bool isEmptyItem(CImage *image)
{
	int pitch = std::abs(image->GetPitch());
	int BPP = image->GetBPP() / 8;
	BYTE *pData = getAddr(*image);

	for (int i = 0; i < image->GetHeight(); i++)
	{
		for (int j = 0; j < image->GetWidth(); j++)
		{
			if (memcmp(pData, pData + (i * pitch) + j * BPP, BPP) != 0)
				return false;
		}
	}
	return true;
}

bool getGameBoard(std::vector<std::vector<int>> &gameBoard)
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
	int nBitPerPixel = GetDeviceCaps(hdcWindow, BITSPIXEL);

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

	// 释放图片上下文
	image.ReleaseDC();
	// 将图片以 BMP 的格式保存到 F:\ScreenShot.bmp
	image.Save(_T("./ScreenShot.bmp"), Gdiplus::ImageFormatBMP);

	vector<std::shared_ptr<CImage> > containerCImages;
	gameBoard = std::vector<std::vector<int> >(Rows, std::vector<int>(Colunms, 0));
	for (int i = 0; i < Rows; i++)
	{
		for (int j = 0; j < Colunms; j++)
		{
			shared_ptr<CImage> ptrImage(new CImage());
			ptrImage->Create(ItemWidth - 4, ItemHeight - 4, nBitPerPixel);
			BitBlt(
				ptrImage->GetDC(),													// 保存到的目标 图片对象 上下文
				0, 0,																// 起始 x, y 坐标
				ItemWidth - 4, ItemHeight - 4,										// 截图宽高
				hdcWindow,															// 截取对象的 上下文句柄
				StartPosX + (ItemWidth * j), StartPosY + (ItemHeight * i) + 4,		// 4 是为了去掉边框的影响
				SRCCOPY);
			ptrImage->ReleaseDC();

			//_tprintf(_T("addr = %p\n"), (BYTE *)ptrImage->GetPixelAddress(0, 0));

			auto pos = std::find_if(begin(containerCImages), end(containerCImages), [&ptrImage](std::shared_ptr<CImage> &value) {
				BYTE *pDataA = getAddr(*ptrImage.get());
				BYTE *pDataB = getAddr(*value.get());

				int bufSize = value->GetHeight() * value->GetWidth() * value->GetBPP() / 8;

				assert(pDataA != pDataB);
				return memcmp(pDataA, pDataB, bufSize) == 0;
			});

			if (pos == end(containerCImages)) {

				//TCHAR filename[MAX_PATH] = { 0 };
				//_stprintf_s(filename, _T("./%02d-%02d.bmp"), i, j);
				//ptrImage->Save(filename, Gdiplus::ImageFormatBMP);

				// 是否是空白项
				bool emptyItem = isEmptyItem(ptrImage.get());

				//if (emptyItem) {
				//	_tprintf(_T("empty ->%s\n"), filename);
				//}

				if (emptyItem && containerCImages.size() > 0) {
					containerCImages.push_back(*containerCImages.begin());
					*containerCImages.begin() = ptrImage;

					// 将已经设置的值回退
					for (int k = 0; k < i * Colunms + j; k++) {
						if (gameBoard[k / Colunms][k % Colunms] == 0)
							gameBoard[k / Colunms][k % Colunms] = containerCImages.size() - 1;
					}

					gameBoard[i][j] = 0;
				}
				else {
					containerCImages.push_back(ptrImage);
					gameBoard[i][j] = containerCImages.size() - 1;
				}
			}
			else
			{
				gameBoard[i][j] = std::distance(begin(containerCImages), pos);
			}
		}
	}

	// 释放dc句柄
	ReleaseDC(hwnd, hdcWindow);

	//// display
	//for (int i = 0; i < Rows; i++)
	//{
	//	for (int j = 0; j < Colunms; j++)
	//	{
	//		_tprintf(_T("%02d,"), gameBoard[i][j]);
	//	}
	//	_tprintf(_T("\n"));
	//}

	return true;
}


int main()
{
	//std::vector<std::vector<int>> board = {
	//	{5,1,2,3,4,3,0,0,0,0,0,0,0,6,7,8,5,9,10,},
	//	{11,12,9,0,0,0,1,0,0,0,0,0,13,0,0,0,6,14,15,},
	//	{0,16,17,3,0,0,18,10,0,0,0,19,11,0,0,20,2,21,0,},
	//	{0,0,16,4,21,0,22,23,12,0,7,17,24,0,3,2,22,0,0,},
	//	{0,0,0,24,23,25,22,13,0,0,0,7,19,26,7,25,0,0,0,},
	//	{0,0,0,0,18,6,17,0,0,0,0,0,15,5,8,0,0,0,0,},
	//	{0,0,0,13,12,27,21,17,0,0,0,4,14,15,23,6,0,0,0,},
	//	{0,0,4,28,21,0,2,25,19,0,18,11,18,0,8,29,16,0,0},
	//	{0,30,16,28,0,0,24,25,0,0,0,1,14,0,0,26,10,19,0,},
	//	{27,30,23,0,0,0,1,0,0,0,0,0,24,0,0,0,5,8,22,},
	//	{15,28,29,11,10,26,0,0,0,0,0,0,0,13,26,12,14,28,20,},
	//};

	//std::vector<std::vector<int>> board = {
	//	{0,0,0,0,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,},
	//	{0,1,2,3,4,5,6,7,8,9,3,10,11,02,12,13,14,15,0, },
	//	{00,16,17,18,19,20,21,06,22,23,12,16,24,10,06,25,24,8,00,},
	//	{00,23,20,26,27,28,29,30,29,31,32,14,33,25,34,10,33,14,00,},
	//	{00,34,31,07,31,10,35,07,16,12,36,01,35,25,05,17,01,36,00,},
	//	{00,04,03,8,26,37,38,33,07,00,34,36,29,23,9,32,38,22,00,},
	//	{00,19,15,15,39,38,40,35,20,02,21,36,01,16,21,39,06,11,00,},
	//	{00,22,9,18,29,11,02,11,40,23,39,20,27,04,30,13,13,13,00,},
	//	{00,12,25,31,14,37,03,33,30,41,04,18,38,30,17,18,41,24,00,},
	//	{00,8,34,22,21,17,24,19,28,28,35,37,9,28,39,15,37,19,00,},
	//	{00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,},
	//};

	bool stop = false;
	while (!stop) {
		std::vector<std::vector<int>> board;
		jpp::Timer timer;
		if (!PerceptualHash::getGameBoard(board)) {
		//if (!::getGameBoard(board)) {
			_tprintf(_T("getData faile!\n"));
			return 0;
		}
		cout << "cost: " << timer.elapse() << "(ms)" << endl;
		timer.reset();

		// display
		for (int i = 0; i < Rows; i++)
		{
			for (int j = 0; j < Colunms; j++)
			{
				_tprintf(_T("%02d,"), board[i][j]);
			}
			_tprintf(_T("\n"));
		}

		CalLinkGame calLinkGame(board, Rows, Colunms);

		if (!calLinkGame.init(programTitle)) {
			_tprintf(_T("CalLinkGame failed!\n"));
			return 0;
		}

		std::vector<std::pair<Point, Point>> ret;
		if (calLinkGame.Cal(ret)) {
			
			this_thread::sleep_for(chrono::seconds(2));
			//calLinkGame.ClickedTrainButton();
			//cout << "clicked Train Button" << endl;
			
		}
		else {
			//calLinkGame.ClickedRefreshButton();
		}
	}
	//for (auto pos = ret.begin(); pos != ret.end(); pos++) {
	//	cout << "( " << pos->first.x_ << ", " << pos->first.y_ << " ) -> "
	//		<< "( " << pos->second.x_ << ", " << pos->second.y_ << " )" << endl;

	//	//auto p1 = calPos(pos->first.x_, pos->first.y_);
	//	//auto p2 = calPos(pos->second.x_, pos->second.y_);
	//	//control.Test(p1.first, p1.second);
	//	//control.Test(p2.first, p2.second);

	//	//this_thread::sleep_for(chrono::milliseconds(100));
	//}

    return 0;
}


