CC = gcc
CFLAGS = -Wall -g -std=c99
OBJS =  ftplib.o ftp.o
ftptool: $(OBJS) -lpthread
