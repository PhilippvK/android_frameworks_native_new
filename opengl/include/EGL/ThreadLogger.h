/*
 * TLT.h
 *
 *  Created on: Jun 16, 2015
 *      Author: dominik
 */

#ifndef TLT_H_
#define TLT_H_

//#define LOG_DEST "/sdcard/THREAD_LOGS/schedstats.log"
#define LOG_DEST "/data/local/tmp/thread_logger/schedstats.log"
#define myfifo "/data/local/tmp/thread_logger/thread_logger_FIFO"

void *thread_log(void *in);
void LOG(unsigned int *PID, long long nr);
void copy_file(char *src, char *dest);
void get_pid_dest(unsigned int *PID, char *pid_dest);
void get_TIDs(char *pid_dest, unsigned int *TID_list);
void *write_log(void *arg);
void *get_log(void *arg);
void read_and_parse_file(char *src, unsigned long long *cpu_time);
double diff_time(timespec *T1, timespec *T2);
void poll_FIFO(unsigned int *PID, int *fifo_handle);
void make_fifo();
int open_fifo(int *fifo_handle);

struct arg_struct {
    char *pid_dest;//[128];
    unsigned int *TID;
    unsigned int *PID;
    unsigned long long *cpu_time_ptr;
};

#endif /* TLT_H_ */
