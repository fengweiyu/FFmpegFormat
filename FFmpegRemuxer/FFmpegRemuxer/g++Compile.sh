#! /bin/sh
#-Wl,-Bstatic 会导致编译出错，错误是/usr/bin/ld: cannot find -lgcc_s
#g++ FFmpegRemuxer.cpp -I ./include -L ./lib -Wl,-Bstatic -lavformat -lavcodec -lavutil -o FFmpegRemuxer 
#g++ FFmpegRemuxer1.cpp -I ./include -L . avformat-55.dll avcodec-55.dll avutil-52.dll -o FFmpegRemuxer
 
#g++ FFmpegRemuxer.cpp -I ./include -L ./lib -lavformat -lavcodec -lavutil -lswresample -o FFmpegRemuxer 


#如下这种形式，定义变量会出错，只有放在Makefile中如下形式才可用
DYNAMIC_LIBS = ./lib/libavformat.so.57
DYNAMIC_LIBS += ./lib/libavcodec.so.57
DYNAMIC_LIBS += ./lib/libavutil.so.55
DYNAMIC_LIBS += ./lib/libswresample.so.2
g++ FFmpegRemuxer.cpp -I ./include -rdynamic ${DYNAMIC_LIBS} -o FFmpegRemuxer 