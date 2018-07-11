#include "stdafx.h"
#include "config.h"
#include "CalLinkGame.h"
#include <queue>

#include <iostream>
using namespace std;

const static int MoveMent[4][3] = { {-1, 0, 1}, {0, -1, 2}, {1, 0, 3}, {0, 1, 4}};

CalLinkGame::CalLinkGame(const std::vector<std::vector<int>>& board, int rows, int column)
	: board_(board)
	, rows_(rows)
	, colums_(column)
	, mouseControl_(250)
{

}

CalLinkGame::~CalLinkGame() {
	// �ر�
	mouseControl_.StopAndWait();
}

bool CalLinkGame::init(LPCTSTR programTitle) {
	return mouseControl_.Init(programTitle);
}

bool CalLinkGame::Cal(std::vector<std::pair<Point, Point>> &ret) {
	// �����еĽڵ����
	vector<Point> vec;
	for (int i = 0; i < rows_; i++) {
		for (int j = 0; j < colums_; j++) {
			if (board_[i][j] != 0) {
				vec.push_back(Point{i, j, board_[i][j]});
			}
		}
	}
	cout << "total size = " << vec.size() << endl;

	ret.clear();

	// �᲻���˲���ѭ���� ?
	while (vec.size()) {
		vector<Point> tmp;
		bool flag = false;
		for (auto pos = vec.begin(); pos != vec.end(); ++pos) {
			Point t;
			if (0 == board_[pos->x_][pos->y_])
			{
				// do nothing
				flag = true;
			}
			else if (findConnectOne(*pos, t)) {
				ret.push_back(make_pair(*pos, t));
				board_[pos->x_][pos->y_] = 0;
				board_[t.x_][t.y_] = 0;

				cout << "( " << pos->x_ << ", " << pos->y_ << " ) ( " << t.x_ << ", " << t.y_ << " )" << endl;

				mouseControl_.ClickedAsync(pos->x_, pos->y_);
				mouseControl_.ClickedAsync(t.x_, t.y_);
				flag = true;
			}
			else { // != 0 and can not find way
				tmp.push_back(*pos);
			}
		}
		vec.swap(tmp);
		cout << "size = " << vec.size() << endl;
		if (!flag) {
			_tprintf(_T("need refresh the game now!"));
			break;
		}
	}

	// ��Ϊ�ճ�������,��Ҫ��մ���
	return vec.size() == 0;
}


void CalLinkGame::ClickedTrainButton()
{
	mouseControl_.ClickedAsync(TrainPosX, TrainPosY);
}

void CalLinkGame::ClickedRefreshButton()
{
	mouseControl_.ClickedAsync(TrainPosX, TrainPosY);
}

bool CalLinkGame::findConnectOne(Point s, Point &t) {
	queue<Point> q;

	// ��ʼ�����
	s.direction_ = 0;
	s.mincross_ = 0;
	q.push(s);

	while (!q.empty()) {
		Point p = q.front();
		q.pop();
		for (int i = 0; i < 4; i++) {
			int tx = MoveMent[i][0] + p.x_;
			int ty = MoveMent[i][1] + p.y_;

			if (tx >= 0 && tx < rows_ && ty >= 0 && ty < colums_) {
				// �����෴
				if (p.direction_ != 0 && abs(p.direction_ - MoveMent[i][2]) == 2) {
					continue;
				}
				// ����������
				int mincross = p.mincross_;
				if (p.direction_ != 0 && p.direction_ != MoveMent[i][2]) {
					mincross++;
				}
				if (mincross > 2) {
					continue;
				}

				// value ��ͬ
				if (board_[tx][ty] == s.value_)
				{
					t = Point(tx, ty, board_[tx][ty]);	
					return true;
				}
				// value Ϊ��
				else if (board_[tx][ty] == 0)
				{
					Point tmp(tx, ty, p.value_);
					tmp.mincross_ = mincross;
					tmp.direction_ = MoveMent[i][2];

					q.push(tmp);
				}
			}
		}
	}
	return false;
}