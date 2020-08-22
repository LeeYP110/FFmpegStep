#include <iostream>

extern "C" {
#include "libavformat/avformat.h"
}
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

int main(int argc, char** argv) {
	std::cout << "Test Demux ffmpeg" << std::endl;
	const char* file_path = "test.mp4";

#ifndef _WIN32
	// 初始化封装库
	av_register_all();
#endif // !_WIN32

	// 初始化网络库
	avformat_network_init();

	// 打开解封装
	AVDictionary* opts = nullptr;
	av_dict_set(&opts, "rtsp_transport", "tcp", 0); // rtsp 网络协议
	av_dict_set(&opts, "max_delay", "500", 0); // 网络延时时间

	AVFormatContext *ic = nullptr; // 解封装上下文
	int result = avformat_open_input(&ic,
		file_path,
		nullptr, // 自动选择解封装器
		&opts); // 参数设置，比如rtsp的延时设置
	if (result != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(result, buf, sizeof(buf) - 1);
		std::cout << "open " << file_path << buf << std::endl;
		getchar();
		return -1;
	}
	else
	{
		std::cout << "open " << file_path << " sucess!" << std::endl;
	}

	if (ic != nullptr)
	{
		avformat_close_input(&ic); // 释放封装上下文并置nullptr
	}
	
	return 0;
}