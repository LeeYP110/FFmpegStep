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
	// ��ʼ����װ��
	av_register_all();
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
	audioIndex = av_find_best_stream(ic, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);
	ic->streams[videoIndex];

#endif	





	if (ic != nullptr)
	{
		avformat_close_input(&ic); // �ͷŷ�װ�����Ĳ���nullptr
	}

	
	
	return 0;
}