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
	// ��ʼ����װ��
	av_register_all();
	// ע�������
	avcodec_register_all();
#endif // !_WIN32

	// ��ʼ�������
	avformat_network_init();

	// �򿪽��װ
	AVDictionary* opts = nullptr;
	av_dict_set(&opts, "rtsp_transport", "tcp", 0); // rtsp ����Э��
	av_dict_set(&opts, "max_delay", "500", 0); // ������ʱʱ��

	AVFormatContext *ic = nullptr; // ���װ������
	int result = avformat_open_input(&ic,
		filePath,
		nullptr, // �Զ�ѡ����װ��
		&opts); // �������ã�����rtsp����ʱ����
	if (result != 0)
	{
		char buf[1024] = { 0 };
		av_strerror(result, buf, sizeof(buf) - 1);
		std::cout << "open " << filePath << buf << std::endl;
		getchar();
		return -1;
	}
	std::cout << "open " << filePath << " sucess!" << std::endl;
	
	// ��ȡ����Ϣ
	result = avformat_find_stream_info(ic, nullptr);	
	int64_t totalMs = ic->duration / (AV_TIME_BASE / 1000);// ��ʱ��
	std::cout << totalMs << std::endl;
	av_dump_format(ic, 0, nullptr, 0);// ��ӡ��Ƶ����ϸ��Ϣ

	// ��ȡ����Ƶ����Ϣ��������������
#if 0
	int videoIndex = 0;
	int audioIndex = 0;
	for (int i = 0; i < ic->nb_streams; i++)
	{
		AVStream* as = ic->streams[i];
		as->codecpar->codec_id; // ������id
		as->codecpar->format;   // �����������ʽ
		if (as->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO)
		{
			audioIndex = i;
			std::cout << "��Ƶ��Ϣ��" << i << std::endl;
			std::cout << "as->codecpar->sample_rate: " << as->codecpar->sample_rate << std::endl; // ������
			std::cout << "as->codecpar->channels: " << as->codecpar->channels << std::endl; // ͨ��
			std::cout << as->codecpar->codec_id << std::endl; // ������id
			std::cout << as->codecpar->format << std::endl;   // �����������ʽ

			// һ֡���ݣ��� ��ͨ��������
			std::cout << "as->codecpar->frame_size: " << as->codecpar->frame_size << std::endl;
			// 1024 *2 *2 fps = sample_rate/frame_size;
		}

		if (as->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_VIDEO)
		{
			videoIndex = i;
			std::cout << "��Ƶ��Ϣ��" << i << std::endl;
			std::cout << "as->codecpar->width: " << as->codecpar->width << std::endl; // ��һ���ܻ�ȡ��
			std::cout << "as->codecpar->height: " << as->codecpar->height << std::endl;
			std::cout << "as->avg_frame_rate: " << r2d(as->avg_frame_rate) << std::endl; // ֡�� 
			std::cout << as->codecpar->codec_id << std::endl; // ������id
			std::cout << as->codecpar->format << std::endl;   // �����������ʽ
		}
	}
#else
	int videoIndex = 0;
	int audioIndex = 0;
	// ��ȡ��Ƶ��
	videoIndex = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	audioIndex = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, nullptr, 0);
	

	/////////////////////////////////////��Ƶ������/////////////////////////////////////
	// �ҵ�������
	AVCodec* vcodec = avcodec_find_decoder(ic->streams[videoIndex]->codecpar->codec_id);
	if (vcodec == nullptr)
	{
		std::cout << "not find vcodec" << std::endl;
		return -1;
	}
	std::cout << "find vcodec: " << ic->streams[videoIndex]->codecpar->codec_id << std::endl;

	// ����������������
	AVCodecContext* vc = avcodec_alloc_context3(vcodec);
	
	// ���ý����������Ĳ���
	avcodec_parameters_to_context(vc, ic->streams[videoIndex]->codecpar);
	vc->thread_count = 8; // 8�߳̽���

	// �򿪽�����������
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

	//////////////////////////////////////��Ƶ������////////////////////////////////////
	// �ҵ�������
	AVCodec* acodec = avcodec_find_decoder(ic->streams[audioIndex]->codecpar->codec_id);
	if (acodec == nullptr)
	{
		std::cout << "not find acodec" << std::endl;
		return -1;
	}
	std::cout << "find vcodec: " << ic->streams[audioIndex]->codecpar->codec_id << std::endl;

	// ����������������
	AVCodecContext* ac = avcodec_alloc_context3(acodec);

	// ���ý����������Ĳ���
	avcodec_parameters_to_context(ac, ic->streams[audioIndex]->codecpar);
	ac->thread_count = 8; // 8�߳̽���

	// �򿪽�����������
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
	SwsContext* vctx = nullptr; // ���ظ�ʽ���ߴ�ת��������
	unsigned char* rgb = nullptr;

	// ��Ƶ�ز���������
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
			std::cout << "��Ƶ"<< std::endl;
			cc = vc;
		}
		if (pkg->stream_index == audioIndex)
		{
			std::cout << "��Ƶ " << std::endl;
			cc = ac;
		}
		std::cout << "pkt->size: " << pkg->size << std::endl;
		std::cout << "pkt->pts: " << pkg->pts * r2d(ic->streams[pkg->stream_index]->time_base) * 1000 << std::endl;
		std::cout << "pkt->dts: " << pkg->dts * r2d(ic->streams[pkg->stream_index]->time_base) * 1000 << std::endl;

		// ������Ƶ
		// ����packet�������߳�,send��nullptr����ö��avcodec_receive_frame��ȡ������������
		result = avcodec_send_packet(cc, pkg);				
		av_packet_unref(pkg);// �ͷţ����ü�����1, Ϊ0�ͷſռ�

		if (result != 0)
		{
			char buf[1024] = { 0 };
			av_strerror(result, buf, sizeof(buf) - 1);
			std::cout << "avcodec_send_packet failed: " << buf << std::endl;
			continue;;
		}
		while (true)
		{
			// һ��send���ܶ�Ӧ���frame
			result = avcodec_receive_frame(cc, frame);
			if (result != 0)
			{
				break;
			}

			if (cc == vc)
			{

				vctx = sws_getCachedContext(
					vctx, // ��nullptr ���´���
					frame->width,
					frame->height,
					(AVPixelFormat)frame->format,
					frame->width,
					frame->height,
					AVPixelFormat::AV_PIX_FMT_RGBA,
					SWS_FAST_BILINEAR, // �ߴ�仯�㷨
					nullptr,
					nullptr,
					nullptr);
				if (vctx)
				{
					//std::cout << "����ת�������Ļ�ȡ�򴴽��ɹ�" << std::endl;
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
					std::cout << "�߶ȣ�" << result << std::endl;
				}				
			}

			if (cc == ac)
			{
				if (pcm == nullptr)
				{
					pcm = new unsigned char[frame->nb_samples * 2 * 2]; // �������� * 2 ������С16bit * 2 ͨ����
				}

				uint8_t *data[2] = { 0 };
				data[0] = pcm;

				result = swr_convert(actx,
					data, frame->nb_samples, // ���
					(const uint8_t**)frame->data, frame->nb_samples); // ����
				std::cout << "�ز�����" << result << std::endl;

			}
			//std::cout << "recv frame " << frame->format << " " << frame->linesize[0] << std::endl;
		}		
	}
	av_packet_free(&pkg);


	if (ic != nullptr)
	{
		avformat_close_input(&ic); // �ͷŷ�װ�����Ĳ���nullptr
	}	
	
	return 0;
}