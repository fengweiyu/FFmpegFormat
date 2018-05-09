/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module       : 	FFmpegRemuxer.cpp
* Description       : 	FFmpegRemuxer Demo

��������
Input #0, flv, from 'cuc_ieschool1.flv':
  Metadata:
    metadatacreator : iku
    hasKeyframes    : true
    hasVideo        : true
    hasAudio        : true
    hasMetadata     : true
    canSeekToEnd    : false
    datasize        : 932906
    videosize       : 787866
    audiosize       : 140052
    lasttimestamp   : 34
    lastkeyframetimestamp: 30
    lastkeyframelocation: 886498
    encoder         : Lavf55.19.104
  Duration: 00:00:34.20, start: 0.042000, bitrate: 394 kb/s
    Stream #0:0: Video: h264 (High), yuv420p, 512x288 [SAR 1:1 DAR 16:9], 15.17 fps, 15 tbr, 1k tbn, 30 tbc
    Stream #0:1: Audio: mp3, 44100 Hz, stereo, s16p, 128 kb/s
Output #0, mp4, to 'cuc_ieschool1.mp4':
    Stream #0:0: Video: h264, yuv420p, 512x288 [SAR 1:1 DAR 16:9], q=2-31, 90k tbn, 30 tbc
    Stream #0:1: Audio: mp3, 44100 Hz, stereo, s16p, 128 kb/s
Write        0 frames to output file
Write        1 frames to output file
Write        2 frames to output file
Write        3 frames to output file
.
.
.

* Created           : 	2017.09.21.
* Author            : 	Yu Weifeng
* Function List     : 	
* Last Modified     : 	
* History           : 	
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
#include <stdio.h>


/*
__STDC_LIMIT_MACROS and __STDC_CONSTANT_MACROS are a workaround to allow C++ programs to use stdint.h
macros specified in the C99 standard that aren't in the C++ standard. The macros, such as UINT8_MAX, INT64_MIN,
and INT32_C() may be defined already in C++ applications in other ways. To allow the user to decide
if they want the macros defined as C99 does, many implementations require that __STDC_LIMIT_MACROS
and __STDC_CONSTANT_MACROS be defined before stdint.h is included.

This isn't part of the C++ standard, but it has been adopted by more than one implementation.
*/
#define __STDC_CONSTANT_MACROS


#ifdef _WIN32//Windows
extern "C"
{
    #include "libavformat/avformat.h"
};
#else//Linux...
    #ifdef __cplusplus
    extern "C"
    {
    #endif
        #include <libavformat/avformat.h>
    #ifdef __cplusplus
    };
    #endif
#endif

/*****************************************************************************
-Fuction        : main
-Description    : main
-Input          : 
-Output         : 
-Return         : 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int main(int argc, char* argv[])
{
	AVOutputFormat * ptOutputFormat = NULL;//The output container format.Muxing only, must be set by the caller before avformat_write_header().
	AVFormatContext * ptInFormatContext = NULL;//�����ļ��ķ�װ��ʽ�����ģ��ڲ��������е���Ƶ��Ϣ
	AVFormatContext * ptOutFormatContext = NULL;//����ļ��ķ�װ��ʽ�����ģ��ڲ��������е���Ƶ��Ϣ
	AVPacket tOutPacket ={0};//�洢һ֡ѹ���������ݸ�����ļ�
	const char * strInFileName=NULL, * strOutFileName = NULL;//�����ļ���������ļ���
	int iRet, i;
	int iFrameCount = 0;//�����֡����
    AVStream * ptInStream=NULL,* ptOutStream=NULL;//��������Ƶ�����������Ƶ��
	
    if(argc!=3)//argc����argv[0]Ҳ���ǳ�������
    {
        printf("Usage:%s InputFileURL OutputFileURL\r\n",argv[0]);
        printf("For example:\r\n");
        printf("%s InputFile.flv OutputFile.mp4\r\n",argv[0]);
        return -1;
    }
	strInFileName = argv[1];//Input file URL
	strOutFileName = argv[2];//Output file URL

	av_register_all();//ע��FFmpeg�������	
	
	/*------------Input------------*/
	if ((iRet = avformat_open_input(&ptInFormatContext, strInFileName, 0, 0)) < 0) 
	{//��������Ƶ�ļ�
		printf("Could not open input file\r\n");
	}
	else
	{
    	if ((iRet = avformat_find_stream_info(ptInFormatContext, 0)) < 0) 
    	{//��ȡ��Ƶ�ļ���Ϣ
    		printf("Failed to find input stream information\r\n");
    	}
    	else
    	{
            av_dump_format(ptInFormatContext, 0, strInFileName, 0);//�ֹ����Եĺ���,�ڲ���log�������صĸ�ʽ��Ϣ��log����
            
            /*------------Output------------*/
            
            /*��ʼ��һ�����������AVFormatContext�ṹ��
             *ctx���������óɹ�֮�󴴽���AVFormatContext�ṹ�塣
             *oformat��ָ��AVFormatContext�е�AVOutputFormat������ȷ�������ʽ�����ָ��ΪNULL��
              �����趨������������format_name����filename����FFmpeg�²������ʽ��
              PS��ʹ�øò�����Ҫ�Լ��ֶ���ȡAVOutputFormat�������ʹ�ú�����������˵Ҫ�鷳һЩ��
             *format_name��ָ�������ʽ�����ơ����ݸ�ʽ���ƣ�FFmpeg���Ʋ������ʽ�������ʽ�����ǡ�flv������mkv���ȵȡ�
             *filename��ָ������ļ������ơ������ļ����ƣ�FFmpeg���Ʋ������ʽ���ļ����ƿ����ǡ�xx.flv������yy.mkv���ȵȡ�
             ����ִ�гɹ��Ļ����䷵��ֵ���ڵ���0
             */
            avformat_alloc_output_context2(&ptOutFormatContext, NULL, NULL, strOutFileName);
            if (!ptOutFormatContext) 
            {
                printf("Could not create output context\r\n");
                iRet = AVERROR_UNKNOWN;
            }
            else
            {
                ptOutputFormat = ptOutFormatContext->oformat;
                for (i = 0; i < ptInFormatContext->nb_streams; i++) 
                {
                    //Create output AVStream according to input AVStream
                    ptInStream = ptInFormatContext->streams[i];
                    ptOutStream = avformat_new_stream(ptOutFormatContext, ptInStream->codec->codec);//��ptOutFormatContext�е�������streams�е�
                    if (!ptOutStream) //һ����(�����е�Ԫ��)����ռ䣬Ҳ����������������˿ռ�,��������ֱ�ӿ�����������(pkt)�Ϳ����ˡ�
                    {
                        printf("Failed allocating output stream\r\\n");
                        iRet = AVERROR_UNKNOWN;
                        break;
                    }
                    else
                    {
                        if (avcodec_copy_context(ptOutStream->codec, ptInStream->codec) < 0) //Copy the settings of AVCodecContext
                        {
                            printf("Failed to copy context from input to output stream codec context\r\n");
                            iRet = AVERROR_UNKNOWN;
                            break;
                        }
                        else
                        {
                            ptOutStream->codec->codec_tag = 0;
                            if (ptOutFormatContext->oformat->flags & AVFMT_GLOBALHEADER)
                                ptOutStream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
                                                        
                        }
                    }
                }
                if(AVERROR_UNKNOWN == iRet)
                {
                }
                else
                {
                    av_dump_format(ptOutFormatContext, 0, strOutFileName, 1);//Output information------------------
                    //Open output file
                    if (!(ptOutputFormat->flags & AVFMT_NOFILE))
                    {   /*��FFmpeg����������ļ�,ʹ������д��������ִ��
                         *s���������óɹ�֮�󴴽���AVIOContext�ṹ�塣
                         *url���������Э��ĵ�ַ���ļ�Ҳ��һ�֡����塱��Э�飬�����ļ���˵�����ļ���·������
                         *flags���򿪵�ַ�ķ�ʽ������ѡ��ֻ����ֻд�����߶�д��ȡֵ���¡�
                                 AVIO_FLAG_READ��ֻ����AVIO_FLAG_WRITE��ֻд��AVIO_FLAG_READ_WRITE����д��*/
                        iRet = avio_open(&ptOutFormatContext->pb, strOutFileName, AVIO_FLAG_WRITE);
                        if (iRet < 0) 
                        {
                            printf("Could not open output file %s\r\n", strOutFileName);
                        }
                        else
                        {
                            //Write file header
                            if (avformat_write_header(ptOutFormatContext, NULL) < 0) //avformat_write_header()����ؼ��ĵط����ǵ�����AVOutputFormat��write_header()
                            {//��ͬ��AVOutputFormat�в�ͬ��write_header()��ʵ�ַ���
                                printf("Error occurred when opening output file\r\n");
                            }
                            else
                            {
                                while (1) 
                                {
                                    //Get an AVPacket
                                    iRet = av_read_frame(ptInFormatContext, &tOutPacket);//�������ļ���ȡһ֡ѹ������
                                    if (iRet < 0)
                                        break;
                                        
                                    ptInStream = ptInFormatContext->streams[tOutPacket.stream_index];
                                    ptOutStream = ptOutFormatContext->streams[tOutPacket.stream_index];
                                    //Convert PTS/DTS
                                    tOutPacket.pts = av_rescale_q_rnd(tOutPacket.pts, ptInStream->time_base, ptOutStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                                    tOutPacket.dts = av_rescale_q_rnd(tOutPacket.dts, ptInStream->time_base, ptOutStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                                    tOutPacket.duration = av_rescale_q(tOutPacket.duration, ptInStream->time_base, ptOutStream->time_base);
                                    tOutPacket.pos = -1;
                                    //Write
                                    /*av_interleaved_write_frame����interleave_packet()�Լ�write_packet()������δ�����AVPacket�������
                                     *write_packet()������ؼ��ĵط����ǵ�����AVOutputFormat��д�����ݵķ�����write_packet()ʵ������һ������ָ�룬
                                     ָ���ض���AVOutputFormat�е�ʵ�ֺ���*/
                                    if (av_interleaved_write_frame(ptOutFormatContext, &tOutPacket) < 0) 
                                    {
                                        printf("Error muxing packet\r\n");
                                        break;
                                    }
                                    printf("Write %8d frames to output file\r\n", iFrameCount);
                                    av_free_packet(&tOutPacket);//�ͷſռ�
                                    iFrameCount++;
                                }
                                //Write file trailer//av_write_trailer()����ؼ��ĵط����ǵ�����AVOutputFormat��write_trailer()
                                av_write_trailer(ptOutFormatContext);//��ͬ��AVOutputFormat�в�ͬ��write_trailer()��ʵ�ַ���
                            }
                            if (ptOutFormatContext && !(ptOutputFormat->flags & AVFMT_NOFILE))
                                avio_close(ptOutFormatContext->pb);//�ú������ڹر�һ��AVFormatContext->pb��һ��������Ǻ�avio_open()�ɶ�ʹ�õġ�
                        }
                    }
                }
                avformat_free_context(ptOutFormatContext);//�ͷſռ�
            }
    	}
        avformat_close_input(&ptInFormatContext);//�ú������ڹر�һ��AVFormatContext��һ��������Ǻ�avformat_open_input()�ɶ�ʹ�õġ�
	}
	return 0;
}
