before run FFmpegMuxer,must export LD_LIBRARY_PATH=./lib

eg:
book@book-desktop:/work/project/FFmpegMuxer$ make clean;make
rm FFmpegMuxer
g++ FFmpegMuxer.cpp -I ./include -rdynamic ./lib/libavformat.so.57 ./lib/libavcodec.so.57 ./lib/libavutil.so.55 ./lib/libswresample.so.2 -o FFmpegMuxer 
book@book-desktop:/work/project/FFmpegMuxer$ export LD_LIBRARY_PATH=./lib
book@book-desktop:/work/project/FFmpegMuxer$ ./FFmpegMuxer sintel.h264 sintel.mp4


