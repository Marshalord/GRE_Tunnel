#!/bin/sh
rm main
gcc -I/usr/include/mysql greController.c socketController.c ipExchange.c mysqlController.c main.c -L/usr/lib/mysql -lmysqlclient -o main -lpthread
