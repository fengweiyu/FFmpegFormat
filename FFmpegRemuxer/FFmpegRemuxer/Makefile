#! /bin/sh
#-Wl,-Bstatic 会导致编译出错，错误是/usr/bin/ld: cannot find -lgcc_s
#g++ FFmpegRemuxer.cpp -I ./include -L ./lib -Wl,-Bstatic -lavformat -lavcodec -lavutil -o FFmpegRemuxer 
#g++ FFmpegRemuxer1.cpp -I ./include -L . avformat-55.dll avcodec-55.dll avutil-52.dll -o FFmpegRemuxer 
#g++ FFmpegRemuxer.cpp -I ./include -L ./lib -lavformat -lavcodec -lavutil -lswresample -o FFmpegRemuxer 

DYNAMIC_LIBS = ./lib/libavformat.so.57
#去掉后面三个库也能完成重新封装的功能(但是去掉上面这个就不行就编译出错)，但是这是由于.lib下还有这三个库的原因，删掉.lib下这三个就不行了,所以不能缺少这四个库
DYNAMIC_LIBS += ./lib/libavcodec.so.57
DYNAMIC_LIBS += ./lib/libavutil.so.55
DYNAMIC_LIBS += ./lib/libswresample.so.2

#-rdynamic选项新添加的是动态连接符号信息，用于动态连接功能，比如dlopen()系列函数、backtrace()系列函数使用，不能被strip掉，即强制strip将导致程序无法执行
#-rdynamic选项不产生任何调试信息，因此在一般情况下，新增的附加信息比-g选项要少得多。除非是完全的静态连接，否则即便是没有加-rdynamic选项，程序使用到的外部动态符号，比如前面示例里的printf，也会被自动加入到.dynsym表
#或者可以使用-Wl,-Bdynamic ${DYN_LIBS}的形式来链接动态库
FFmpegRemuxer : FFmpegRemuxer.cpp
	g++ $^ -I ./include -rdynamic ${DYNAMIC_LIBS} -o $@ 
	
clean:
	rm FFmpegRemuxer