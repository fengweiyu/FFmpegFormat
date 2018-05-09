/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module       : 	FFmpegMuxer.cpp
* Description       : 	FFmpegMuxer Demo

*�Ƚ�H.264�ļ������ڴ棬 
*�������װ��ʽ�ļ���

��������
book@book-desktop:/work/project/FFmpegMuxer$ make clean;make
rm FFmpegMuxer
g++ FFmpegMuxer.cpp -I ./include -rdynamic ./lib/libavformat.so.57 ./lib/libavcodec.so.57 ./lib/libavutil.so.55 ./lib/libswresample.so.2 -o FFmpegMuxer 
book@book-desktop:/work/project/FFmpegMuxer$ export LD_LIBRARY_PATH=./lib
book@book-desktop:/work/project/FFmpegMuxer$ ./FFmpegMuxer sintel.h264 sintel.mp4 
Input #0, h264, from 'sintel.h264':
  Duration: N/A, bitrate: N/A
    Stream #0:0: Video: h264 (High), yuv420p(progressive), 640x360, 25 fps, 25 tbr, 1200k tbn, 50 tbc
Output #0, mp4, to 'sintel.mp4':
    Stream #0:0: Unknown: none
[mp4 @ 0x9352d80] Using AVStream.codec.time_base as a timebase hint to the muxer is deprecated. Set AVStream.time_base instead.
[mp4 @ 0x9352d80] Using AVStream.codec to pass codec parameters to muxers is deprecated, use AVStream.codecpar instead.
Write iFrameIndex:1,stream_index:0,num:25,den:1
Write iFrameIndex:2,stream_index:0,num:25,den:1
Write iFrameIndex:3,stream_index:0,num:25,den:1
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

#define IO_BUFFER_SIZE 32768  //����32k
  
static FILE * g_fileH264=NULL;

 
/*****************************************************************************
-Fuction        : FillIoBuffer
-Description    : FillIoBuffer

*��avformat_open_input()�л��״ε��øûص������� 
*�ڶ���һֱ�����һ�ζ�����avformat_find_stream_info()��ѭ�����ã� 
*�ļ��е�����ÿ��IO_BUFFER_SIZE�ֽڶ��뵽�ڴ��У� 
*����ffmpeg�����������ݱ��������֡�洢��AVPacketList�С� 
*�����ǻ�����Ϊ32KB������������С���ò�ͬ�����û���Ҳ������ͬ��

-Input          : 
-Output         : 
-Return         : ���ض�ȡ�ĳ���
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int FillIoBuffer(void *opaque, unsigned char *o_pbBuf, int i_iMaxSize)  
{ 
    int iRet=-1;
    if (!feof(g_fileH264))
    {  
        iRet = fread(o_pbBuf, 1, i_iMaxSize, g_fileH264);  
    }  
    else
    {  
    }  
    return iRet;  
}  

/*****************************************************************************
-Fuction        : main
-Description    : main
�ؼ�Ҫ��avformat_open_input()֮ǰ��ʼ��һ��AVIOContext��
���ҽ�ԭ����AVFormatContext��ָ��pb��AVIOContext���ͣ�ָ��������г�ʼ��AVIOContext
-Input          : 
-Output         : 
-Return         : 
* Modify Date	  Version		 Author 		  Modification
* -----------------------------------------------
* 2017/09/21	  V1.0.0		 Yu Weifeng 	  Created
******************************************************************************/
int main(int argc, char* argv[])
{
	AVInputFormat * ptInputFormat = NULL;//The output container format.Muxing only, must be set by the caller before avformat_write_header().
	AVOutputFormat * ptOutputFormat = NULL;//The output container format.Muxing only, must be set by the caller before avformat_write_header().
	AVFormatContext * ptInFormatContext = NULL;//�����ļ��ķ�װ��ʽ�����ģ��ڲ��������е���Ƶ��Ϣ
	AVFormatContext * ptOutFormatContext = NULL;//����ļ��ķ�װ��ʽ�����ģ��ڲ��������е���Ƶ��Ϣ
	AVPacket tOutPacket ={0};//�洢һ֡ѹ���������ݸ�����ļ�
	const char * strInVideoFileName=NULL, * strOutFileName = NULL;//�����ļ���������ļ���
	int iRet, i;
    int iVideoStreamIndex = -1;//��Ƶ��Ӧ�ô��ڵ�λ��
	int iFrameIndex = 0;
    long long llCurrentPts = 0;  
    int iOutVideoStreamIndex = -1; //������е���Ƶ�����ڵ�λ��
    AVStream * ptInStream=NULL,* ptOutStream=NULL;//��������Ƶ�����������Ƶ��
	unsigned char * pbIoBuf=NULL;//io���ݻ�����
	AVIOContext * ptAVIO=NULL;//AVIOContext��������������ݵĽṹ��
	
    if(argc!=3)//argc����argv[0]Ҳ���ǳ�������
    {
        printf("Usage:%s InputVideoFileURL OutputFileURL\r\n",argv[0]);
        printf("For example:\r\n");
        printf("%s InputFile.h264 OutputFile.mp4\r\n",argv[0]);
        return -1;
    }
	strInVideoFileName = argv[1];//Input file URL
	strOutFileName = argv[2];//Output file URL

	av_register_all();//ע��FFmpeg�������	
	
	/*------------Input:���ptInFormatContext------------*/
    g_fileH264 = fopen(strInVideoFileName, "rb+");  
    ptInFormatContext = avformat_alloc_context();  
    pbIoBuf = (unsigned char *)av_malloc(IO_BUFFER_SIZE);  
    //FillIoBuffer���ǽ����ݶ�ȡ��pbIoBuf�Ļص�������FillIoBuffer()��ʽ������������ֵ���ǹ̶��ģ���һ���ص�����,
    ptAVIO = avio_alloc_context(pbIoBuf, IO_BUFFER_SIZE, 0, NULL, FillIoBuffer, NULL, NULL);  //��ϵͳ��Ҫ���ݵ�ʱ�򣬻��Զ����øûص������Ի�ȡ����
    ptInFormatContext->pb = ptAVIO; //������ָ����AVIOContext֮��avformat_open_input()�����URL�����Ͳ���������
    
    ptInputFormat = av_find_input_format("h264");//�õ�ptInputFormat�Ա�����ʹ��
    //ps:�������óɹ�֮�������AVFormatContext�ṹ��;file:�򿪵�����Ƶ�����ļ�·��������ý��URL;fmt:ǿ��ָ��AVFormatContext��AVInputFormat��,ΪNULL,FFmpegͨ���ļ�·��������ý��URL�Զ����;dictionay:���ӵ�һЩѡ��,һ������¿�������ΪNULL
    //�ڲ���Ҫ��������������init_input()�����󲿷ֳ�ʼ�������������������ġ�s->iformat->read_header()����ȡ��ý�������ļ�ͷ����������Ƶ��������Ӧ��AVStream
	if ((iRet = avformat_open_input(&ptInFormatContext, "", ptInputFormat, NULL)) < 0) //���е�init_input()���ָ����fmt(����������,���統ǰ����ָ��)��ֱ�ӷ��أ����û��ָ���͵���av_probe_input_buffer2()�Ʋ�AVInputFormat
	{//��������ƵԴ//�Զ����˻ص�����FillIoBuffer()����ʹ��avformat_open_input()��ý�����ݵ�ʱ�򣬾Ϳ��Բ�ָ���ļ���URL�ˣ������2������ΪNULL����Ϊ���ݲ��ǿ��ļ���ȡ��������FillIoBuffer()�ṩ��
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
            av_dump_format(ptInFormatContext, 0, strInVideoFileName, 0);//�ֹ����Եĺ���,�ڲ���log�������صĸ�ʽ��Ϣ��log����
            
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
                //for (i = 0; i < ptInFormatContext->nb_streams; i++) 
                {
                    //Create output AVStream according to input AVStream
                    ptInStream = ptInFormatContext->streams[0];//0 video
                    ptOutStream = avformat_new_stream(ptOutFormatContext, ptInStream->codec->codec);//��ptOutFormatContext�е�������streams�е�
                    if (!ptOutStream) //һ����(�����е�Ԫ��)����ռ䣬Ҳ����������������˿ռ�,��������ֱ�ӿ�����������(pkt)�Ϳ����ˡ�
                    {
                        printf("Failed allocating output stream\r\\n");
                        iRet = AVERROR_UNKNOWN;
                        //break;
                    }
                    else
                    {
                        iVideoStreamIndex=0;
                        iOutVideoStreamIndex = ptOutStream->index; //������Ƶ�����������λ�� 
                        if (avcodec_copy_context(ptOutStream->codec, ptInStream->codec) < 0) //Copy the settings of AVCodecContext
                        {//avcodec_copy_context()�������Խ�������Ƶ/��Ƶ�Ĳ��������������Ƶ/��Ƶ��AVCodecContext�ṹ��
                            printf("Failed to copy context from input to output stream codec context\r\n");
                            iRet = AVERROR_UNKNOWN;
                            //break;
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
                                    int iStreamIndex = -1;//���ڱ�ʶ��ǰ���ĸ���  
                                    iStreamIndex = iOutVideoStreamIndex;
                                    //Get an AVPacket//����Ƶ��������ȡ����Ƶ��AVPacket
                                    iRet = av_read_frame(ptInFormatContext, &tOutPacket);//�������ļ���ȡһ֡ѹ������
                                    if (iRet < 0)
                                        break;
                                    else
                                    {
                                        do{  
                                            ptInStream = ptInFormatContext->streams[tOutPacket.stream_index];
                                            ptOutStream = ptOutFormatContext->streams[iStreamIndex];
                                            if (tOutPacket.stream_index == iVideoStreamIndex)
                                            { //H.264����û��PTS����˱����ֶ�д��PTS,Ӧ�÷���av_read_frame()֮��
                                                //FIX��No PTS (Example: Raw H.264)  
                                                //Simple Write PTS  
                                                if (tOutPacket.pts == AV_NOPTS_VALUE)
                                                {  
                                                    //Write PTS  
                                                    AVRational time_base1 = ptInStream->time_base;  
                                                    //Duration between 2 frames (��s)     ������25֡����֡���40ms //AV_TIME_BASE��ʾ1s�����������ĵ�λΪus��Ҳ����ffmpeg�ж���us 
                                                    //int64_t calc_duration = AV_TIME_BASE*1/25;//��40*1000;//(double)AV_TIME_BASE / av_q2d(ptInStream->r_frame_rate);//ptInStream->r_frame_rate.den����0����ע�͵�  
                                                    //֡��Ҳ���Դ�h264�����л�ȡ,ǰ��dump�������,���ǲ�֪��Ϊ��ͬ���ı���ǰ��r_frame_rate��ӡ����,����ʹ�õ�ʱ��ȴ�������ˣ�����������ʱ��ֻ��ʹ��avg_frame_rate���߸��ݼ���֡����д
                                                    int64_t calc_duration =(double)AV_TIME_BASE / av_q2d(ptInStream->avg_frame_rate);
                                                    //Parameters    pts(��ʾʱ���)*pts��λ(ʱ���*ʱ�����λ)=��ʵ��ʾ��ʱ��(��ν֡����ʾʱ�䶼����Ե�һ֡����)
                                                    tOutPacket.pts = (double)(iFrameIndex*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);//AV_TIME_BASEΪ1s�������䵥λΪus
                                                    tOutPacket.dts = tOutPacket.pts;  
                                                    tOutPacket.duration = (double)calc_duration / (double)(av_q2d(time_base1)*AV_TIME_BASE);  
                                                    iFrameIndex++;  
                                                    printf("Write iFrameIndex:%d,stream_index:%d,num:%d,den:%d\r\n",iFrameIndex, tOutPacket.stream_index,ptInStream->avg_frame_rate.num,ptInStream->avg_frame_rate.den);  
                                                }  
                                                llCurrentPts = tOutPacket.pts;  
                                                break;  
                                            }  
                                        } while (av_read_frame(ptInFormatContext, &tOutPacket) >= 0);  
                                    }
                                        
                                    //Convert PTS/DTS
                                    tOutPacket.pts = av_rescale_q_rnd(tOutPacket.pts, ptInStream->time_base, ptOutStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                                    tOutPacket.dts = av_rescale_q_rnd(tOutPacket.dts, ptInStream->time_base, ptOutStream->time_base, (AVRounding)(AV_ROUND_NEAR_INF | AV_ROUND_PASS_MINMAX));
                                    tOutPacket.duration = av_rescale_q(tOutPacket.duration, ptInStream->time_base, ptOutStream->time_base);
                                    tOutPacket.pos = -1;
                                    tOutPacket.stream_index = iStreamIndex; 
                                    //printf("Write 1 Packet. size:%5d\tpts:%lld\n", tOutPacket.size, tOutPacket.pts);  
                                    //Write
                                    /*av_interleaved_write_frame����interleave_packet()�Լ�write_packet()������δ�����AVPacket�������
                                     *write_packet()������ؼ��ĵط����ǵ�����AVOutputFormat��д�����ݵķ�����write_packet()ʵ������һ������ָ�룬
                                     ָ���ض���AVOutputFormat�е�ʵ�ֺ���*/
                                    if (av_interleaved_write_frame(ptOutFormatContext, &tOutPacket) < 0) 
                                    {
                                        printf("Error muxing packet\r\n");
                                        break;
                                    }
                                    av_free_packet(&tOutPacket);//�ͷſռ�
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
	if(NULL!=g_fileH264)
        fclose(g_fileH264);  
	return 0;
}
