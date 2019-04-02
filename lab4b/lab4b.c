
#include <stdio.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <poll.h>
#include <getopt.h>
#include <math.h>
#include <mraa.h>
#include <sys/time.h>
#include <sys/types.h>
#include <errno.h>
#include <time.h>
#include <ctype.h>
#include "fcntl.h"

int shut = 1;
int AIO_PIN_1 = 1;
int GPIO_PIN_1 = 60;
const int B = 4275;                 // B value of the thermistor
const float R0 = 100000.0; 
char scale = 'F'; //defaults to Farhanheit           // R0 = 100k
int generate; 
int logger_fd;
char* logger_file;
int logger_flag = -1;

int period;
struct timeval clock_x;

struct tm* now;

void handleInterrupt(){
    shut = 1;
}

double getTemp(int tempReading){
    int hunn_thou = 100000;
    double x = (1023.0 / (double)tempReading - 1.0) * hunn_thou;
    float temp = 1.0/(log(x/hunn_thou)/B+1/298.15) - 273.15;
    if(scale == 'C'){
        temp = temp*9/5 + 32;
    }
    return temp;
}

void report(double temp){
    time_t timex;
    struct tm* current;
    time(&timex);
    current = localtime(&timex);
    fprintf(stdout, "%.2d:%.2d:%.2d %.1f\n", current->tm_hour, current->tm_min, current->tm_sec, temp);
    if(generate) {
    	dprintf(logger_fd, "%.2d:%.2d:%.2d %.1f\n", current->tm_hour, current->tm_min, current->tm_sec, temp);
    }
}
void handleInput(const char* input){
    printf("input = %s \n", input);
    if(strcmp(input, "OFF") == 0){
        dprintf(logger_fd, "OFF\n");
        now = localtime(&(clock_x.tv_sec));
        fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
        if(logger_flag!=-1)
            dprintf(logger_fd, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
        exit(1);
    }
    else if(strcmp(input, "START") == 0){
        dprintf(logger_fd, "START\n");
        generate = 1;
    }
    else if(strcmp(input, "STOP") == 0){
        dprintf(logger_fd, "STOP\n");
        generate = 0;
    }
    else if(strcmp(input, "SCALE=F") == 0){
        scale = 'F';
        dprintf(logger_fd, "SCALE=F\n");
    }
    else if(strcmp(input, "SCALE=C") == 0){
        scale = 'C';
        dprintf(logger_fd, "SCALE=C\n");
    }
    else if(strncmp(input, "PERIOD=", sizeof(char)*7) == 0){
        period = (int)atoi(input+7);
        if(generate && logger_flag != -1){
            dprintf(logger_fd, "PERIOD=%d\n", period);
        }
    }
    else if(strncmp(input, "LOG",sizeof(char)*3) == 0){
        if(logger_flag != -1){
           dprintf(logger_fd, "%s\n", input);
        }
    }
    else{
        fprintf(stdout, "Invalid command \n");
        //exit(1);
    }
    fflush(stdout);
}
int main(int argc, char ** argv){
    static struct option long_options[] = {
        {"period", required_argument, 0, 'p'},
        {"scale", required_argument, 0, 's' },
        {"stop", no_argument, 0, 'o'},
        {"start", no_argument, 0, 'a'},
        {"log", optional_argument, 0, 'l'},
        {"off", no_argument, 0, 'f'},
        {0,0,0,0}
    };
    //int error_no = 0;
    int option_index = 0;
    period = 0;

    generate = 1;
    int ch;

    while((ch = getopt_long(argc, argv, "", long_options, &option_index)) != -1){
	switch(ch){
        case 'p': //period
            period = atoi(optarg);
            break;
        case 's': //scale
            break;
        case 'o':
            generate = 0;
            break;
        case 'a':
            generate = 1;
            break;
        case 'l':
            logger_flag = 1;
            logger_fd = creat((char*)optarg, 0664);
            printf("%s", (char*)optarg);
            if(logger_fd < 0){
                fprintf(stderr, "error assigning fd. error: %d, message: %s \n", errno, strerror(errno));
                exit(1);
            }
           
            break;
        case 'f':
            dprintf(logger_fd, "OFF\n");
            now = localtime(&(clock_x.tv_sec));
	        fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
            if(logger_flag!=-1)
                dprintf(logger_fd, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
            exit(1);
            break;
        default:
            fprintf(stderr, "Invalid command. \n");
            exit(1);
 	   }
}

    //initializing sensors
    mraa_aio_context temp_sensor = mraa_aio_init(AIO_PIN_1);
    mraa_gpio_context button = mraa_gpio_init(GPIO_PIN_1);
    mraa_gpio_isr(button, MRAA_GPIO_EDGE_RISING, &handleInterrupt, NULL);
    
    //time_t next = 0;
    struct pollfd poll_fd[1];
    poll_fd[0].fd = STDIN_FILENO;
    poll_fd[0].events = POLLIN;// | POLLER | POLLHUP;
    poll_fd[0].revents = 0;

    //char* copy_buf = malloc(sizeof(char)*256);
    //char* cmd_buf = malloc(sizeof(char)*256);
    char copy_buf[256];
    char cmd_buf[256];

    memset(copy_buf, 0, 256);
    memset(cmd_buf, 0, 256);

    int copyIndex = 0;
    for(;;){
        int val = mraa_aio_read(temp_sensor);
        double tempVal = getTemp(val);
        if(generate)
            report(tempVal); //implement
        time_t begin, end;
        time(&begin);
        time(&end);
        while(difftime(end, begin) < period){
            if(mraa_gpio_read(button)){
		    dprintf(logger_fd, "OFF\n");
            	    now = localtime(&(clock_x.tv_sec));
	            fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
           	    if(logger_flag!=-1){
                    	dprintf(logger_fd, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
            	    	exit(1);
	            }
            }
            // button_shutdown();
            if(poll(poll_fd, 1, 0) < 0){
                fprintf(stderr, "error inside poll. error:%d, message:%s \n", errno, strerror(errno));
                exit(1);
            }
            if(poll_fd[0].revents && POLLIN){
                int size = 0;
                if((size = read(STDIN_FILENO, cmd_buf, 256)) < 0){
                    fprintf(stderr, "error inside poll. error:%d, message:%s \n", errno, strerror(errno));
                    exit(1);
                }
		        int i = 0;
                for(i = 0; i < size && copyIndex < 256; i++){
                    if(cmd_buf[i] != '\n'){
                        copy_buf[copyIndex] = cmd_buf[i];
                        copyIndex++; 
                    }
                    else{
                        //printf("copy_buf = %s\n", copy_buf);
                        handleInput((char*)&copy_buf);
                        copyIndex = 0;
                        memset(copy_buf, 0, 256);
                    }
                }
            }    
            time(&end);
        }
    }
    mraa_aio_close(temp_sensor);
    mraa_gpio_close(button);
    close(logger_fd);
    exit(0);
}