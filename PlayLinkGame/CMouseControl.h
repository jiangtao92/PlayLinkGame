#ifndef __CMOUSECONTROL_H_
#define __CMOUSECONTROL_H_

#include "safeQueue.h"
#include "util.h"

#include <future>
#include <atomic>
#include <utility>
#include <windows.h>

class MouseControl : private handy::noncopyable {
public:
	MouseControl(int timeoutMills = 0) : timeoutMills_(timeoutMills), exit_(false) {};

	~MouseControl();
	
	bool Init(LPCTSTR programTitle);

	void ClickedAsync(int xpos, int ypos);

	void Clicked(int xpos, int ypos);

	// wait forever
	void StopAndWait();
private:
	std::pair<int, int> calPos(int i, int j);
private:
	HWND									hwnd_;
	handy::SafeQueue<std::pair<int, int> >	safeQueue_;
	int										timeoutMills_{0};
	std::future<void>						ret_;			// 用于等待异步任务完成
	std::atomic<bool>						exit_{false};		// 等待退出
};
#endif