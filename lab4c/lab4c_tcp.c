
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
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

int shut = 1;
int AIO_PIN_1 = 1;
int GPIO_PIN_1 = 60;
const int B = 4275;                 // B value of the thermistor
const float R0 = 100000.0; 
char scale = 'F'; //defaults to Farhanheit           // R0 = 100k
int generate = 1; 
int logger_fd;
char* logger_file;
int logger_flag = -1;
struct timeval my_clock;
int period = 1;
struct timeval clock_x;
time_t next_time;
struct tm* now;
struct tm* current_time;
mraa_aio_context temp_sensor;
char* my_id;
char* hostname = NULL;
int port_num;
int socket_fd;
struct sockaddr_in server_addr;
struct hostent *server;

void handleInterrupt(){
    shut = 1;
    exit(1);
}

double getTemp(){
 int a = mraa_aio_read(temp_sensor);

  int B = 4275;
  float R0 = 100000.0;
  float R = 1023.0/((float)a)-1.0;
  R = R0*R;

  float temperature = 1.0/(log(R/R0)/B+1/298.15)-273.15;
   if(temperature < 0){
    temperature = temperature * -1;
  }

  return (scale == 'F') ? ((temperature * 9)/5 + 32) : temperature;
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
   // printf("input = %s \n", input);
    if(strcmp(input, "OFF") == 0){
        dprintf(logger_fd, "OFF\n");
        now = localtime(&(clock_x.tv_sec));
        fprintf(stdout, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
        if(logger_flag!=-1)
            dprintf(logger_fd, "%02d:%02d:%02d SHUTDOWN\n", now->tm_hour, now->tm_min, now->tm_sec);
        exit(0);
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
   // fflush(stdout);
}
int main(int argc, char ** argv){
    static struct option long_options[] = {
        {"period", required_argument, 0, 'p'},
        {"scale", required_argument, 0, 's'},
        {"stop", no_argument, 0, 'o'},
        {"start", no_argument, 0, 'a'},
        {"log", optional_argument, 0, 'l'},
        {"off", no_argument, 0, 'f'},
        {"id", required_argument, 0, 'i'},
        {"host", required_argument, 0, 'h'},
        {0,0,0,0}
    };
    int option_index = 0;
    period = 1;
    generate = 1;
    int ch;

    while((ch = getopt_long(argc, argv, "", long_options, &option_index)) != -1){
	    switch(ch){
            case 'p': //period
                period = atoi(optarg);
                break;
            case 's': //scale
                switch(*optarg){
                    case 'C':
                    //case 'c':
                        scale = 'C';
                        break;
                    case 'F':
                    //case 'f':
                        scale = 'F';
                        break;
                }
                if(*optarg != 'F' || *optarg != 'C'){
                    fprintf(stderr, "scale can either be F or C \n");
                    exit(1);
                }
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
              //  printf("%s", (char*)optarg);
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
            case 'i':
                if(strlen(optarg) != 9){
                    fprintf(stderr, "id_length is wrong");
                    exit(0);
                }
                my_id = optarg;
                break;
            case 'h':
                hostname = optarg;
                break;
            default:

                fprintf(stderr, "Invalid command. \n");
                exit(1);
            }
        }
    //initialize TCP
    port_num = atoi(argv[optind]);
    if(port_num <= 0){
        fprintf(stderr, "Invalid port number\n");
        exit(1);
    }
    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if(socket_fd < 0){
        fprintf(stderr, "Unable to assign fd to socket. error: %d, message: %s", errno, strerror(errno));
        exit(2);
    }
    server = gethostbyname(hostname);
    if(server == NULL){
        fprintf(stderr, "Server error. error: %d, message %s", errno, strerror(errno));
        exit(2);
    }
    memset((char*)&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(port_num);
    if(connect(socket_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0){
        fprintf(stderr, "connection error. error: %d, message: %s", errno, strerror(errno));
    }
    dprintf(socket_fd, "ID=%s\n", my_id);
    dprintf(logger_fd, "ID=%s\n", my_id);


    //initializing sensors
    temp_sensor = mraa_aio_init(AIO_PIN_1);
   
    
    //time_t next = 0;
    struct pollfd poll_fd[1];
    poll_fd[0].fd = STDIN_FILENO;
    poll_fd[0].events = POLLIN;
    poll_fd[0].revents = 0;

    //char* copy_buf = malloc(sizeof(char)*256);
    //char* cmd_buf = malloc(sizeof(char)*256);
    char copy_buf[256];
    char cmd_buf[256];

    memset(copy_buf, 0, 256);
    memset(cmd_buf, 0, 256);

    int copyIndex = 0;

    for(;;){

       double tempVal = getTemp();
        if(generate)
            report(tempVal); 
        
        //dprintf(logger_fd, "should be generating report \n");
        time_t begin, end;
        time(&begin);
        time(&end);
        
        while(difftime(end, begin) < period){
            int ret = poll(poll_fd, 1, 0);
            if(ret < 0){
                fprintf(stderr, "error inside poll. error:%d, message:%s \n", errno, strerror(errno));
                exit(1);
            } else if (ret > 0) {
                if(poll_fd[0].revents && POLLIN){
                    int size = read(STDIN_FILENO, cmd_buf, 256);
                    if(size < 0){
                        fprintf(stderr, "read error. error:%d, message:%s \n", errno, strerror(errno));
                        exit(1);
                    }
                    int i;
                    for(i = 0; i < size && copyIndex < 256; i++){
                        if(cmd_buf[i] == '\n'){ 
                           // printf("copy_buf = %s\n", copy_buf);
                            handleInput((char*)&copy_buf);
                            copyIndex = 0;
                            memset(copy_buf, 0, 256);
                        }
                        else{
                            copy_buf[copyIndex] = cmd_buf[i];
                            copyIndex++;
                        }
                    }
                }
            }
            time(&end);
        }
    }
    mraa_aio_close(temp_sensor);
    close(logger_fd);
    exit(0);
}