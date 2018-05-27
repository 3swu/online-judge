all:judger judge-server
	.PHONY:all
judger:judger.c get_process_memsize.h outfile_compare.h error_and_result.h
	gcc judger.c -o judger
judge-server:judge-server.c
	gcc judge-server.c -o judge-server
