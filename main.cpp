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
		file_path,
		nullptr, // �Զ�ѡ����װ��
		&opts); // �������ã�����rtsp����ʱ����
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
		avformat_close_input(&ic); // �ͷŷ�װ�����Ĳ���nullptr
	}
	
	return 0;
}