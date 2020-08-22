#include <iostream>

extern "C" {
#include "libavformat/avformat.h"
}
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

int main(int argc, char** argv) {
	std::cout << "Test Demux ffmpeg" << std::endl;
	const char* filePath = "test.mkv";

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
		filePath,
		nullptr, // 自动选择解封装器
		&opts); // 参数设置，比如rtsp的延时设置
	if (result != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(result, buf, sizeof(buf) - 1);
		std::cout << "open " << filePath << buf << std::endl;
		getchar();
		return -1;
	}
	std::cout << "open " << filePath << " sucess!" << std::endl;
	
	// 获取流信息
	result = avformat_find_stream_info(ic, nullptr);	
	int64_t totalMs = ic->duration / (AV_TIME_BASE / 1000);// 总时长
	std::cout << totalMs << std::endl;
	av_dump_format(ic, 0, nullptr, 0);// 打印视频流详细信息

	// 获取音视频流信息（遍历、函数）
#if 0
	int videoIndex = 0;
	int audioIndex = 0;
	for (int i = 0; i < ic->nb_streams; i++)
	{
		AVStream* as = ic->streams[i];
		as->codecpar->codec_id; // 解码器id
		as->codecpar->format;   // 样本、编码格式
		if (as->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO)
		{
			audioIndex = i;
			std::cout << "音频信息：" << i << std::endl;
			std::cout << "as->codecpar->sample_rate: " << as->codecpar->sample_rate << std::endl; // 样本率
			std::cout << "as->codecpar->channels: " << as->codecpar->channels << std::endl; // 通道
			std::cout << as->codecpar->codec_id << std::endl; // 解码器id
			std::cout << as->codecpar->format << std::endl;   // 样本、编码格式

			// 一帧数据？？ 单通道样本数
			std::cout << "as->codecpar->frame_size: " << as->codecpar->frame_size << std::endl;
			// 1024 *2 *2 fps = sample_rate/frame_size;
		}

		if (as->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO)
		{
			videoIndex = i;
			std::cout << "视频信息：" << i << std::endl;
			std::cout << "as->codecpar->width: " << as->codecpar->width << std::endl; // 不一定能获取到
			std::cout << "as->codecpar->height: " << as->codecpar->height << std::endl;
			std::cout << "as->avg_frame_rate: " << r2d(as->avg_frame_rate) << std::endl; // 帧率 
			std::cout << as->codecpar->codec_id << std::endl; // 解码器id
			std::cout << as->codecpar->format << std::endl;   // 样本、编码格式
		}
	}
#else
	int videoIndex = 0;
	int audioIndex = 0;
	// 获取视频流
	videoIndex = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	audioIndex = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	ic->streams[videoIndex];

#endif	





	if (ic != nullptr)
	{
		avformat_close_input(&ic); // 释放封装上下文并置nullptr
	}

	
	
	return 0;
}