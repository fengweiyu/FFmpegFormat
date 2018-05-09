/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module       : 	FFmpegRemuxer.cpp
* Description       : 	FFmpegRemuxer Demo

输出结果：
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
	AVFormatContext * ptInFormatContext = NULL;//输入文件的封装格式上下文，内部包含所有的视频信息
	AVFormatContext * ptOutFormatContext = NULL;//输出文件的封装格式上下文，内部包含所有的视频信息
	AVPacket tOutPacket ={0};//存储一帧压缩编码数据给输出文件
	const char * strInFileName=NULL, * strOutFileName = NULL;//输入文件名和输出文件名
	int iRet, i;
	int iFrameCount = 0;//输出的帧个数
    AVStream * ptInStream=NULL,* ptOutStream=NULL;//输入音视频流和输出音视频流
	
    if(argc!=3)//argc包括argv[0]也就是程序名称
    {
        printf("Usage:%s InputFileURL OutputFileURL\r\n",argv[0]);
        printf("For example:\r\n");
        printf("%s InputFile.flv OutputFile.mp4\r\n",argv[0]);
        return -1;
    }
	strInFileName = argv[1];//Input file URL
	strOutFileName = argv[2];//Output file URL

	av_register_all();//注册FFmpeg所有组件	
	
	/*------------Input------------*/
	if ((iRet = avformat_open_input(&ptInFormatContext, strInFileName, 0, 0)) < 0) 
	{//打开输入视频文件
		printf("Could not open input file\r\n");
	}
	else
	{
    	if ((iRet = avformat_find_stream_info(ptInFormatContext, 0)) < 0) 
    	{//获取视频文件信息
    		printf("Failed to find input stream information\r\n");
    	}
    	else
    	{
            av_dump_format(ptInFormatContext, 0, strInFileName, 0);//手工调试的函数,内部是log，输出相关的格式信息到log里面
            
            /*------------Output------------*/
            
            /*初始化一个用于输出的AVFormatContext结构体
             *ctx：函数调用成功之后创建的AVFormatContext结构体。
             *oformat：指定AVFormatContext中的AVOutputFormat，用于确定输出格式。如果指定为NULL，
              可以设定后两个参数（format_name或者filename）由FFmpeg猜测输出格式。
              PS：使用该参数需要自己手动获取AVOutputFormat，相对于使用后两个参数来说要麻烦一些。
             *format_name：指定输出格式的名称。根据格式名称，FFmpeg会推测输出格式。输出格式可以是“flv”，“mkv”等等。
             *filename：指定输出文件的名称。根据文件名称，FFmpeg会推测输出格式。文件名称可以是“xx.flv”，“yy.mkv”等等。
             函数执行成功的话，其返回值大于等于0
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
                    ptOutStream = avformat_new_stream(ptOutFormatContext, ptInStream->codec->codec);//给ptOutFormatContext中的流数组streams中的
                    if (!ptOutStream) //一条流(数组中的元素)分配空间，也正是由于这里分配了空间,后续操作直接拷贝编码数据(pkt)就可以了。
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
                    {   /*打开FFmpeg的输入输出文件,使后续读写操作可以执行
                         *s：函数调用成功之后创建的AVIOContext结构体。
                         *url：输入输出协议的地址（文件也是一种“广义”的协议，对于文件来说就是文件的路径）。
                         *flags：打开地址的方式。可以选择只读，只写，或者读写。取值如下。
                                 AVIO_FLAG_READ：只读。AVIO_FLAG_WRITE：只写。AVIO_FLAG_READ_WRITE：读写。*/
                        iRet = avio_open(&ptOutFormatContext->pb, strOutFileName, AVIO_FLAG_WRITE);
                        if (iRet < 0) 
                        {
                            printf("Could not open output file %s\r\n", strOutFileName);
                        }
                        else
                        {
                            //Write file header
                            if (avformat_write_header(ptOutFormatContext, NULL) < 0) //avformat_write_header()中最关键的地方就是调用了AVOutputFormat的write_header()
                            {//不同的AVOutputFormat有不同的write_header()的实现方法
                                printf("Error occurred when opening output file\r\n");
                            }
                            else
                            {
                                while (1) 
                                {
                                    //Get an AVPacket
                                    iRet = av_read_frame(ptInFormatContext, &tOutPacket);//从输入文件读取一帧压缩数据
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
                                    /*av_interleaved_write_frame包括interleave_packet()以及write_packet()，将还未输出的AVPacket输出出来
                                     *write_packet()函数最关键的地方就是调用了AVOutputFormat中写入数据的方法。write_packet()实际上是一个函数指针，
                                     指向特定的AVOutputFormat中的实现函数*/
                                    if (av_interleaved_write_frame(ptOutFormatContext, &tOutPacket) < 0) 
                                    {
                                        printf("Error muxing packet\r\n");
                                        break;
                                    }
                                    printf("Write %8d frames to output file\r\n", iFrameCount);
                                    av_free_packet(&tOutPacket);//释放空间
                                    iFrameCount++;
                                }
                                //Write file trailer//av_write_trailer()中最关键的地方就是调用了AVOutputFormat的write_trailer()
                                av_write_trailer(ptOutFormatContext);//不同的AVOutputFormat有不同的write_trailer()的实现方法
                            }
                            if (ptOutFormatContext && !(ptOutputFormat->flags & AVFMT_NOFILE))
                                avio_close(ptOutFormatContext->pb);//该函数用于关闭一个AVFormatContext->pb，一般情况下是和avio_open()成对使用的。
                        }
                    }
                }
                avformat_free_context(ptOutFormatContext);//释放空间
            }
    	}
        avformat_close_input(&ptInFormatContext);//该函数用于关闭一个AVFormatContext，一般情况下是和avformat_open_input()成对使用的。
	}
	return 0;
}
