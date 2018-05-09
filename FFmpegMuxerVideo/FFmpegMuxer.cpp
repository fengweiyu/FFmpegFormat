/*****************************************************************************
* Copyright (C) 2017-2020 Hanson Yu  All rights reserved.
------------------------------------------------------------------------------
* File Module       : 	FFmpegMuxer.cpp
* Description       : 	FFmpegMuxer Demo

*先将H.264文件读入内存， 
*再输出封装格式文件。

输出结果：
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

#define IO_BUFFER_SIZE 32768  //缓存32k
  
static FILE * g_fileH264=NULL;

 
/*****************************************************************************
-Fuction        : FillIoBuffer
-Description    : FillIoBuffer

*在avformat_open_input()中会首次调用该回调函数， 
*第二次一直到最后一次都是在avformat_find_stream_info()中循环调用， 
*文件中的数据每次IO_BUFFER_SIZE字节读入到内存中， 
*经过ffmpeg处理，所有数据被有序地逐帧存储到AVPacketList中。 
*以上是缓存设为32KB的情况，缓存大小设置不同，调用机制也有所不同。

-Input          : 
-Output         : 
-Return         : 返回读取的长度
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
关键要在avformat_open_input()之前初始化一个AVIOContext，
而且将原本的AVFormatContext的指针pb（AVIOContext类型）指向这个自行初始化AVIOContext
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
	AVFormatContext * ptInFormatContext = NULL;//输入文件的封装格式上下文，内部包含所有的视频信息
	AVFormatContext * ptOutFormatContext = NULL;//输出文件的封装格式上下文，内部包含所有的视频信息
	AVPacket tOutPacket ={0};//存储一帧压缩编码数据给输出文件
	const char * strInVideoFileName=NULL, * strOutFileName = NULL;//输入文件名和输出文件名
	int iRet, i;
    int iVideoStreamIndex = -1;//视频流应该处在的位置
	int iFrameIndex = 0;
    long long llCurrentPts = 0;  
    int iOutVideoStreamIndex = -1; //输出流中的视频流所在的位置
    AVStream * ptInStream=NULL,* ptOutStream=NULL;//输入音视频流和输出音视频流
	unsigned char * pbIoBuf=NULL;//io数据缓冲区
	AVIOContext * ptAVIO=NULL;//AVIOContext管理输入输出数据的结构体
	
    if(argc!=3)//argc包括argv[0]也就是程序名称
    {
        printf("Usage:%s InputVideoFileURL OutputFileURL\r\n",argv[0]);
        printf("For example:\r\n");
        printf("%s InputFile.h264 OutputFile.mp4\r\n",argv[0]);
        return -1;
    }
	strInVideoFileName = argv[1];//Input file URL
	strOutFileName = argv[2];//Output file URL

	av_register_all();//注册FFmpeg所有组件	
	
	/*------------Input:填充ptInFormatContext------------*/
    g_fileH264 = fopen(strInVideoFileName, "rb+");  
    ptInFormatContext = avformat_alloc_context();  
    pbIoBuf = (unsigned char *)av_malloc(IO_BUFFER_SIZE);  
    //FillIoBuffer则是将数据读取至pbIoBuf的回调函数。FillIoBuffer()形式（参数，返回值）是固定的，是一个回调函数,
    ptAVIO = avio_alloc_context(pbIoBuf, IO_BUFFER_SIZE, 0, NULL, FillIoBuffer, NULL, NULL);  //当系统需要数据的时候，会自动调用该回调函数以获取数据
    ptInFormatContext->pb = ptAVIO; //当自行指定了AVIOContext之后，avformat_open_input()里面的URL参数就不起作用了
    
    ptInputFormat = av_find_input_format("h264");//得到ptInputFormat以便后面打开使用
    //ps:函数调用成功之后处理过的AVFormatContext结构体;file:打开的视音频流的文件路径或者流媒体URL;fmt:强制指定AVFormatContext中AVInputFormat的,为NULL,FFmpeg通过文件路径或者流媒体URL自动检测;dictionay:附加的一些选项,一般情况下可以设置为NULL
    //内部主要调用两个函数：init_input()：绝大部分初始化工作都是在这里做的。s->iformat->read_header()：读取多媒体数据文件头，根据视音频流创建相应的AVStream
	if ((iRet = avformat_open_input(&ptInFormatContext, "", ptInputFormat, NULL)) < 0) //其中的init_input()如果指定了fmt(第三个参数,比如当前就有指定)就直接返回，如果没有指定就调用av_probe_input_buffer2()推测AVInputFormat
	{//打开输入视频源//自定义了回调函数FillIoBuffer()。在使用avformat_open_input()打开媒体数据的时候，就可以不指定文件的URL了，即其第2个参数为NULL（因为数据不是靠文件读取，而是由FillIoBuffer()提供）
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
            av_dump_format(ptInFormatContext, 0, strInVideoFileName, 0);//手工调试的函数,内部是log，输出相关的格式信息到log里面
            
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
                //for (i = 0; i < ptInFormatContext->nb_streams; i++) 
                {
                    //Create output AVStream according to input AVStream
                    ptInStream = ptInFormatContext->streams[0];//0 video
                    ptOutStream = avformat_new_stream(ptOutFormatContext, ptInStream->codec->codec);//给ptOutFormatContext中的流数组streams中的
                    if (!ptOutStream) //一条流(数组中的元素)分配空间，也正是由于这里分配了空间,后续操作直接拷贝编码数据(pkt)就可以了。
                    {
                        printf("Failed allocating output stream\r\\n");
                        iRet = AVERROR_UNKNOWN;
                        //break;
                    }
                    else
                    {
                        iVideoStreamIndex=0;
                        iOutVideoStreamIndex = ptOutStream->index; //保存视频流所在数组的位置 
                        if (avcodec_copy_context(ptOutStream->codec, ptInStream->codec) < 0) //Copy the settings of AVCodecContext
                        {//avcodec_copy_context()函数可以将输入视频/音频的参数拷贝至输出视频/音频的AVCodecContext结构体
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
                                    int iStreamIndex = -1;//用于标识当前是哪个流  
                                    iStreamIndex = iOutVideoStreamIndex;
                                    //Get an AVPacket//从视频输入流中取出视频的AVPacket
                                    iRet = av_read_frame(ptInFormatContext, &tOutPacket);//从输入文件读取一帧压缩数据
                                    if (iRet < 0)
                                        break;
                                    else
                                    {
                                        do{  
                                            ptInStream = ptInFormatContext->streams[tOutPacket.stream_index];
                                            ptOutStream = ptOutFormatContext->streams[iStreamIndex];
                                            if (tOutPacket.stream_index == iVideoStreamIndex)
                                            { //H.264裸流没有PTS，因此必须手动写入PTS,应该放在av_read_frame()之后
                                                //FIX：No PTS (Example: Raw H.264)  
                                                //Simple Write PTS  
                                                if (tOutPacket.pts == AV_NOPTS_VALUE)
                                                {  
                                                    //Write PTS  
                                                    AVRational time_base1 = ptInStream->time_base;  
                                                    //Duration between 2 frames (μs)     。假设25帧，两帧间隔40ms //AV_TIME_BASE表示1s，所以用它的单位为us，也就是ffmpeg中都是us 
                                                    //int64_t calc_duration = AV_TIME_BASE*1/25;//或40*1000;//(double)AV_TIME_BASE / av_q2d(ptInStream->r_frame_rate);//ptInStream->r_frame_rate.den等于0所以注释掉  
                                                    //帧率也可以从h264的流中获取,前面dump就有输出,但是不知道为何同样的变量前面r_frame_rate打印正常,这里使用的时候却不正常了，所以这个间隔时间只能使用avg_frame_rate或者根据假设帧率来写
                                                    int64_t calc_duration =(double)AV_TIME_BASE / av_q2d(ptInStream->avg_frame_rate);
                                                    //Parameters    pts(显示时间戳)*pts单位(时间基*时间基单位)=真实显示的时间(所谓帧的显示时间都是相对第一帧来的)
                                                    tOutPacket.pts = (double)(iFrameIndex*calc_duration) / (double)(av_q2d(time_base1)*AV_TIME_BASE);//AV_TIME_BASE为1s，所以其单位为us
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
                                    /*av_interleaved_write_frame包括interleave_packet()以及write_packet()，将还未输出的AVPacket输出出来
                                     *write_packet()函数最关键的地方就是调用了AVOutputFormat中写入数据的方法。write_packet()实际上是一个函数指针，
                                     指向特定的AVOutputFormat中的实现函数*/
                                    if (av_interleaved_write_frame(ptOutFormatContext, &tOutPacket) < 0) 
                                    {
                                        printf("Error muxing packet\r\n");
                                        break;
                                    }
                                    av_free_packet(&tOutPacket);//释放空间
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
	if(NULL!=g_fileH264)
        fclose(g_fileH264);  
	return 0;
}
