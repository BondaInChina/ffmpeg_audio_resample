#include <iostream>
#include "util.h"

extern "C"
{
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}


using namespace std;

#ifdef _WIN32
static const char *path = "E:\\test.mp4";
static const char *pcmPath = "E:\\test.pcm";
#else
static const char *path = "/home/test.mp4";
static const char *pcmPath = "/home/liangjf/test.pcm";
#endif

int main(int argc, char *argv[])
{
    av_register_all();
    avformat_network_init();
    avcodec_register_all();

    AVDictionary *opt = NULL;
    av_dict_set(&opt, "rtsp_transport", "tcp", 0);
    av_dict_set(&opt,"max_delay", "500", 0);

    AVFormatContext *pFormatCtx = NULL;
    int ret = avformat_open_input(&pFormatCtx, path, NULL, &opt);
    if(ret != 0)
    {
        cout << "open " << path << " failed!" << endl;
        getchar();
        return -1;
    }

    ret = avformat_find_stream_info(pFormatCtx, NULL);

    //av_dump_format(pFormatCtx, 0, path, 0);

    int videoIndex = -1;
    int audioIndex = -1;
    for(int i = 0; i < pFormatCtx->nb_streams; i++)
    {
        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO)
        {
            videoIndex = i;
        }

        if(pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
        {
            audioIndex = i;
        }
    }

    if(videoIndex == -1)
    {
        cout << "can not find video stream" << endl;
        avformat_close_input(&pFormatCtx);
        getchar();
        return -1;
    }

    if(audioIndex == -1)
    {
        cout << "can not find audio stream" << endl;
        avformat_close_input(&pFormatCtx);
        getchar();
        return -1;
    }

    AVStream *videoStream = pFormatCtx->streams[videoIndex];
    AVStream *audioStream = pFormatCtx->streams[audioIndex];

    cout << "audio duration: " << audioStream->duration *avio_r2d(audioStream->time_base) << endl;
    cout << "audio bit_rate: " << audioStream->codecpar->bit_rate << endl;
    cout << "audio channels: " << audioStream->codecpar->channels << endl;
    cout << "audio sample_rate: " << audioStream->codecpar->sample_rate << endl;

    AVCodec *vCodec = avcodec_find_decoder(videoStream->codecpar->codec_id);
    if(vCodec == NULL)
    {
        cout << "can not find decoder: " << videoStream->codecpar->codec_id << endl;
        avformat_close_input(&pFormatCtx);
        getchar();
        return -1;
    }

    AVCodecContext *vCodecCtx = avcodec_alloc_context3(vCodec);
    ret = avcodec_open2(vCodecCtx, vCodec, NULL);
    if(ret != 0)
    {
        cout << "open video codec failed!" << endl;
        avcodec_free_context(&vCodecCtx);
        avformat_close_input(&pFormatCtx);
        getchar();
        return -1;
    }

    AVCodec *aCodec = avcodec_find_decoder(audioStream->codecpar->codec_id);
    if(aCodec == NULL)
    {
        cout << "can not find decoder: " << audioStream->codecpar->codec_id << endl;
        avcodec_free_context(&vCodecCtx);
        avformat_close_input(&pFormatCtx);
        getchar();
        return -1;
    }

    AVCodecContext *aCodecCtx = audioStream->codec;
    ret = avcodec_open2(aCodecCtx, aCodec, NULL);
    if(ret != 0)
    {
        cout << "open audio codec failed!" << endl;
        avcodec_close(aCodecCtx);
        avcodec_free_context(&aCodecCtx);
        avformat_close_input(&pFormatCtx);
        getchar();
        return -1;
    }

    SwrContext *swrCtx = swr_alloc();
    swrCtx = swr_alloc_set_opts(swrCtx,
                                av_get_default_channel_layout(2),
                                AV_SAMPLE_FMT_S16,
                                aCodecCtx->sample_rate,
                                av_get_default_channel_layout(aCodecCtx->channels),
                                aCodecCtx->sample_fmt,
                                aCodecCtx->sample_rate,
                                0,
                                NULL);
    ret = swr_init(swrCtx);
    if(ret != 0)
    {
        cout<< "swr_init failed!" << endl;
        avcodec_close(aCodecCtx);
        avcodec_free_context(&vCodecCtx);
        avformat_close_input(&pFormatCtx);
        swr_free(&swrCtx);
        getchar();
        return -1;//wei zuo neicun xielou chuli
    }

    AVPacket *pkt = av_packet_alloc();
    AVFrame *frame = av_frame_alloc();
    uint8_t *pcm = NULL;
    while(1)
    {
        if(av_read_frame(pFormatCtx, pkt) != 0)
        {
            break;
        }
        if(pkt->stream_index == audioIndex)
        {
            int got_frame = 0;
            avcodec_decode_audio4(aCodecCtx, frame, &got_frame, pkt);
            if(got_frame)
            {
                if(pcm == NULL)
                {
                    pcm = new uint8_t[frame->nb_samples * frame->channels * 2];
                }
                ret = swr_convert(swrCtx, &pcm, frame->nb_samples, (const uint8_t**)frame->data, frame->nb_samples);
                write_file(pcmPath, pcm, frame->nb_samples * frame->channels * 2);
            }
        }
    }


    av_frame_free(&frame);
    av_packet_free(&pkt);
    avcodec_close(aCodecCtx);
    swr_free(&swrCtx);
    avcodec_free_context(&vCodecCtx);
    avformat_close_input(&pFormatCtx);

    cout<< "#############################################" << endl;
    cout<< "Test Successfully!" << endl;
    cout<< "#############################################" << endl;
    getchar();
    return 0;
}
