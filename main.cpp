#include <iostream>

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
}
#pragma comment(lib, "avformat.lib")
#pragma comment(lib, "avutil.lib")
#pragma comment(lib, "avcodec.lib")
#pragma comment(lib, "swscale.lib")
#pragma comment(lib, "swresample.lib")

static double r2d(AVRational r)
{
	return r.den == 0 ? 0 : (double)r.num / (double)r.den;
}

int main(int argc, char** argv) {
	std::cout << "Test Demux ffmpeg" << std::endl;
	const char* filePath = "test.mp4";

#ifndef _WIN32
	// 初始化封装库
	av_register_all();
	// 注册解码器
	avcodec_register_all();
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
	audioIndex = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	

	/////////////////////////////////////视频解码器/////////////////////////////////////
	// 找到解码器
	AVCodec* vcodec = avcodec_find_decoder(ic->streams[videoIndex]->codecpar->codec_id);
	if (vcodec == nullptr)
	{
		std::cout << "not find vcodec" << std::endl;
		return -1;
	}
	std::cout << "find vcodec: " << ic->streams[videoIndex]->codecpar->codec_id << std::endl;

	// 创建解码器上下文
	AVCodecContext* vc = avcodec_alloc_context3(vcodec);
	
	// 配置解码器上下文参数
	avcodec_parameters_to_context(vc, ic->streams[videoIndex]->codecpar);
	vc->thread_count = 8; // 8线程解码

	// 打开解码器上下文
	result = avcodec_open2(vc, nullptr, nullptr);
	if (result != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(result, buf, sizeof(buf) - 1);
		std::cout << "video avcodec_open2 failed: " << buf << std::endl;
		getchar();
		return -1;
	}
	std::cout << "video avcodec_open2 success" << std::endl;

	//////////////////////////////////////音频解码器////////////////////////////////////
	// 找到解码器
	AVCodec* acodec = avcodec_find_decoder(ic->streams[audioIndex]->codecpar->codec_id);
	if (acodec == nullptr)
	{
		std::cout << "not find acodec" << std::endl;
		return -1;
	}
	std::cout << "find vcodec: " << ic->streams[audioIndex]->codecpar->codec_id << std::endl;

	// 创建解码器上下文
	AVCodecContext* ac = avcodec_alloc_context3(acodec);

	// 配置解码器上下文参数
	avcodec_parameters_to_context(ac, ic->streams[audioIndex]->codecpar);
	ac->thread_count = 8; // 8线程解码

	// 打开解码器上下文
	result = avcodec_open2(ac, nullptr, nullptr);
	if (result != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(result, buf, sizeof(buf) - 1);
		std::cout << "audio avcodec_open2 failed: " << buf << std::endl;
		getchar();
		return -1;
	}
	std::cout << "audio avcodec_open2 success" << std::endl;

#endif
	AVPacket* pkg = av_packet_alloc();
	AVFrame* frame = av_frame_alloc();
	SwsContext* vctx = nullptr; // 像素格式、尺寸转换上下文
	unsigned char* rgb = nullptr;

	// 音频重采样上下文
	SwrContext* actx = swr_alloc();
	actx = swr_alloc_set_opts(actx,
		av_get_default_channel_layout(2),
		AV_SAMPLE_FMT_S16,
		ac->sample_rate,
		av_get_default_channel_layout(ac->channels),
		ac->sample_fmt,
		ac->sample_rate,
		0,
		nullptr);
	result = swr_init(actx);
	if (result != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(result, buf, sizeof(buf) - 1);
		std::cout << "swr_init failed: " << buf << std::endl;
		getchar();
		return -1;
	}

	unsigned char* pcm = nullptr;
	while (true)
	{		
		result = av_read_frame(ic, pkg);
		if (result != 0)
		{
			int ms = 3000;
			long long pos = (double)ms / (double)1000 / r2d(ic->streams[pkg->stream_index]->time_base);
			av_seek_frame(ic, videoIndex, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
			continue;
		}

		AVCodecContext* cc  = nullptr;
		if (pkg->stream_index == videoIndex)
		{
			std::cout << "视频"<< std::endl;
			cc = vc;
		}
		if (pkg->stream_index == audioIndex)
		{
			std::cout << "音频 " << std::endl;
			cc = ac;
		}
		std::cout << "pkt->size: " << pkg->size << std::endl;
		std::cout << "pkt->pts: " << pkg->pts * r2d(ic->streams[pkg->stream_index]->time_base) * 1000 << std::endl;
		std::cout << "pkt->dts: " << pkg->dts * r2d(ic->streams[pkg->stream_index]->time_base) * 1000 << std::endl;

		// 解码视频
		// 发送packet到解码线程,send传nullptr后调用多次avcodec_receive_frame，取出缓冲中数据
		result = avcodec_send_packet(cc, pkg);				
		av_packet_unref(pkg);// 释放，引用计数减1, 为0释放空间

		if (result != 0)
		{
			char buf[1024] = { 0 };
			av_strerror(result, buf, sizeof(buf) - 1);
			std::cout << "avcodec_send_packet failed: " << buf << std::endl;
			continue;;
		}
		while (true)
		{
			// 一次send可能对应多个frame
			result = avcodec_receive_frame(cc, frame);
			if (result != 0)
			{
				break;
			}

			if (cc == vc)
			{

				vctx = sws_getCachedContext(
					vctx, // 传nullptr 会新创建
					frame->width,
					frame->height,
					(AVPixelFormat)frame->format,
					frame->width,
					frame->height,
					AVPixelFormat::AV_PIX_FMT_RGBA,
					SWS_FAST_BILINEAR, // 尺寸变化算法
					nullptr,
					nullptr,
					nullptr);
				if (vctx)
				{
					//std::cout << "像素转换上下文获取或创建成功" << std::endl;
					if (rgb == nullptr)
					{
						rgb = new unsigned char[frame->width * frame->height * 4];
					}

					uint8_t *data[2] = { 0 };
					data[0] = rgb;
					int lines[2] = { 0 };
					lines[0] = frame->width * 4;
					
					result = sws_scale(vctx,
						frame->data,
						frame->linesize,
						0,
						frame->height,
						data,
						lines);
					std::cout << "高度：" << result << std::endl;
				}				
			}

			if (cc == ac)
			{
				if (pcm == nullptr)
				{
					pcm = new unsigned char[frame->nb_samples * 2 * 2]; // 样本个数 * 2 样本大小16bit * 2 通道数
				}

				uint8_t *data[2] = { 0 };
				data[0] = pcm;

				result = swr_convert(actx,
					data, frame->nb_samples, // 输出
					(const uint8_t**)frame->data, frame->nb_samples); // 输入
				std::cout << "重采样：" << result << std::endl;

			}
			//std::cout << "recv frame " << frame->format << " " << frame->linesize[0] << std::endl;
		}		
	}
	av_packet_free(&pkg);


	if (ic != nullptr)
	{
		avformat_close_input(&ic); // 释放封装上下文并置nullptr
	}	
	
	return 0;
}