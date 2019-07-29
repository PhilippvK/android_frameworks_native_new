/*
 * TLT.cpp
 *
 *  Created on: Jun 16, 2015
 *      Author: dominik
 */
#include <stdio.h>
//#include <iostream>
#include <stdlib.h>
#include <sys/sendfile.h>
#include <string.h>
#include <dirent.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <android/log.h>
#include "EGL/ThreadLogger.h"
#include "EGL/cpufreq_gamegovernor_eglapi.h"
#include "EGL/eglAPI_GOV.h"

FILE *fp_logfile;
timespec T_START;

void *thread_log(void *in) {
    static long long frame_nr;
    static long long call_nr;
    static unsigned int PID;
    unsigned int PID_buf;
    static int init, init_logfile;
    static int fifo_handle;
    static timespec time_stamp;
    timespec time_now;

    call_nr++;

    LOGI("PID before: %u\n",PID);
    PID=getpid();
    LOGI("PID after: %u\n",PID);
    
    if (init==0 || init==2) {//not initialized yet! || init=2 => not initialized but thread blocked for opening
        make_fifo();
        init=2;
        if(open_fifo(&fifo_handle)) {
            init=1; //init succesfull!
            LOGI("Init succesfull!\n");
        }
        else {
            init=0;
            return NULL;
        }
    }


    if (init_logfile==0) {
        fp_logfile=fopen(LOG_DEST, "a");
        if(fp_logfile==NULL) {
            printf("Could not open %s > %s\n", LOG_DEST, strerror(errno));
            init_logfile=0;
            return NULL;
        }
        else {
            fprintf(fp_logfile, "PID, Frame-Nr, TIME since start, Time to get Log-Data, TID, CPU-Time, TID, CPU-Time, ...");
            init_logfile=1;
            LOGI("Opened Logfile succesfully!\n");
        }
    }


    /*if (PID!=0 && diff_time(&time_now, &time_stamp)>5) {
        time_stamp=time_now;
        umask(0777);
        LOGI("LOG!!!! Frame-Nr.: %lld \n", frame_nr);
        LOG(&PID, frame_nr);
    }*/
    umask(0777); // TODO: test
    if (PID!=0) {
        LOG(&PID, frame_nr);
    }

    PID_buf=PID;
    poll_FIFO(&PID, &fifo_handle);
    if(PID_buf!=PID) {
        LOGI("New PID: %u", PID);
        frame_nr=0;
        clock_gettime(CLOCK_MONOTONIC, &T_START);
    }
    else {
        frame_nr++;
    }

    return NULL;
}


void LOG(unsigned int *PID, long long nr) {
    timespec t_start_log;
    clock_gettime(CLOCK_MONOTONIC, &t_start_log);

    unsigned int TID_list[512]={0};
    int nr_threads=0;
    char pid_dest[128];


    get_pid_dest(PID, pid_dest);
    get_TIDs(pid_dest, TID_list);
    //arg.PID=PID;

    //count number of threads
    while(TID_list[nr_threads]!=0) {
        nr_threads++;
    }

    unsigned long long cpu_time[nr_threads];
    arg_struct arg[nr_threads];//argument-struct for log-threads
    for(int i=0; i<nr_threads; i++) {
        arg[i].pid_dest=pid_dest;
        arg[i].TID=&(TID_list[i]);;
        arg[i].PID=PID;
        arg[i].cpu_time_ptr=&(cpu_time[i]);;
    }

    //get log of each thread
    for(int i=0; i<nr_threads; i++) {
        get_log((void *)&arg[i]);
    }

    timespec t_get;
    clock_gettime(CLOCK_MONOTONIC, &t_get);

    //get time since start
    double delta_start=diff_time(&t_get, &t_start_log);
    double delta_frame=diff_time(&t_get, &T_START);

    //write log data
    fprintf(fp_logfile, "%u,%lld,%f,%f,", *PID, nr, delta_frame, delta_start);
    for(int i=0; i<nr_threads; i++) {
        write_log((void *)&arg[i]);
    }

    fprintf(fp_logfile, "\n");

    //timespec t_total;
    //double delta_total;
    //clock_gettime(CLOCK_MONOTONIC, &t_total);
    //delta_total=diff_time(&t_total, &t_start);
    //printf("Time TOTAL: %f\n", delta_total);
}


void  get_pid_dest(unsigned int *PID, char *pid_dest){
    char PID_str[8];
    sprintf(PID_str, "%u", *PID);
    strcpy (pid_dest, "/proc/");
    strcat (pid_dest, PID_str);
    strcat (pid_dest, "/task/");
}


void get_TIDs(char *pid_dest, unsigned int *TID_list) {
    DIR *dir;
    int i=0;
    struct dirent *ent;

    if ((dir = opendir (pid_dest)) != NULL) {
      while ((ent = readdir (dir)) != NULL) {
        if (i>=2) { //2 offset to skip . and ..
            TID_list[i-2]=(unsigned int)atol(ent->d_name);
        }
        i++;
      }
      closedir (dir);
    } else {
      /* could not open directory */
      printf("Error opening dir: %s", pid_dest);
    }
}


void *write_log(void *arg) {
    struct arg_struct *args = (struct arg_struct *)arg;
    char *pid_dest=args->pid_dest;
    unsigned int *TID=args->TID;
    unsigned long long *cpu_time_ptr=args->cpu_time_ptr;

    fprintf(fp_logfile, "%u,%llu,",*TID, *cpu_time_ptr);

    return NULL;
}


void *get_log(void *arg) {
    struct arg_struct *args = (struct arg_struct *)arg;

    char TID_str[8];
    char TID_dest[128];
    char TID_schedstat[128];

    sprintf(TID_str, "%u", *args->TID);
    strcpy(TID_schedstat, args->pid_dest);
    strcat(TID_schedstat, TID_str);
    strcat(TID_schedstat, "/schedstat");

    read_and_parse_file(TID_schedstat, args->cpu_time_ptr);

    return NULL;
}


void read_and_parse_file(char *src, unsigned long long *cpu_time){
    int i=0;
    char dst[16]="\0";
    char buf;
    FILE *fp;

    fp=fopen(src, "r");
    if(fp==NULL) {
        printf("Could not open %s > %s\n", src, strerror(errno));
        return;
    }

    while( ( buf = fgetc(fp) ) !=' ' ) {
        dst[i]=buf;
        i++;
    }

    *cpu_time=strtoull(dst, (char **)NULL, 10);
    fclose(fp);
 }


double diff_time(timespec *T1, timespec *T2) {
    double diffT=((double)T1->tv_sec+1.0e-9*(double)T1->tv_nsec)-
                            ((double)T2->tv_sec+1.0e-9*(double)T2->tv_nsec);
    return diffT;
}

void poll_FIFO(unsigned int *PID, int *fifo_handle) {
    int fifo;
    char str[16];
    int n_bytes;

    //fifo=open(myfifo, O_RDONLY | O_NONBLOCK);
    //if(fifo) {
    //    LOGE("Could not open %s > %s\n", myfifo, strerror(errno));
    //    return;
    //}

    n_bytes=read(*fifo_handle,str,sizeof(str));
    //close(fifo);

    //LOGI("Nr Bytes: %d\n", n_bytes);

    if (n_bytes>0){
        *PID=(unsigned int)atol(str);
        LOGI("New PID: %d\n", *PID);
    }
}

void make_fifo() {
    if(mkfifo(myfifo, 0666)) {
            if (errno!=17) {
                LOGE("Error making dir: %s -> %s\n", myfifo, strerror(errno));
            }
        }
}

int open_fifo(int *fifo_handle) {
    *fifo_handle=open(myfifo, O_NONBLOCK | O_RDONLY);
    if(*fifo_handle==-1) {
        LOGE("Could not open %s > %s\n", myfifo, strerror(errno));
        return 0;
    }
    else {
        return 1;
    }
}

