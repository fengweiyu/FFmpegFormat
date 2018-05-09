before run FFmpegRemuxer,must export LD_LIBRARY_PATH=./lib

eg:
book@book-desktop:/work/project/FFmpegRemuxer$ make clean;make
book@book-desktop:/work/project/FFmpegRemuxer$ export LD_LIBRARY_PATH=./lib
book@book-desktop:/work/project/FFmpegRemuxer$ ./FFmpegRemuxer cuc_ieschool1.flv cuc_ieschool1.mp4 

