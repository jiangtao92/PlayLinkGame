#ifndef __CALLINKGAME_H_
#define __CALLINKGAME_H_

#include <vector>
#include "CMouseControl.h"

// todo: 实现从index想坐标点的转换
struct Point {
	Point(int x = 0, int y = 0, int value = 0) : x_(x), y_(y), value_ (value) {};
	int x_;
	int y_;
	int value_;
	int mincross_{ 0 };
	int direction_{ 0 };
};

class CalLinkGame {
public:
	// TODO: 参数校验
	CalLinkGame(const std::vector<std::vector<int>>& board, int rows, int column);
	
	~CalLinkGame();

	bool init(LPCTSTR programTitle);

	bool Cal(std::vector<std::pair<Point, Point>> & ret);

	void ClickedTrainButton();

	void ClickedRefreshButton();

private:
	bool findConnectOne(Point s, Point &t);

private:
	std::vector<std::vector<int>> board_;
	int rows_;
	int colums_;
	MouseControl mouseControl_;
};

#endif
