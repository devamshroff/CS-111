#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <getopt.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>

int errno;

// --rdonly, --wronly, --command, and --verbose.
/*
void checkfdsize(int *fd_factor, int *fd_count, int* fd_array){
  if(fd_factor < fd_count - 1){
    fd_factor = fd_factor + 5;
    fd_array = (int*)realloc(fd_array, sizeof(int*)*(fd_factor));
  }
  }*/

int main(int argc, char** argv){
    int ch;
    int fd_factor = 10;
    int* fd_array = (int*)malloc(sizeof(int*)*fd_factor);
    int fd_count = 0;
    int verbose = 0;
    static struct option long_options[] =
    {
	{"rdonly", required_argument, 0, 'a'},
	{"wronly", required_argument, 0, 'b'},
	{"command", required_argument, 0, 'c'},
	{"verbose", no_argument, 0, 'd'},
	{0,0,0,0}
    };
    int error_no = 0;
   int option_index = 0;
   while((ch = getopt_long(argc, argv, "a:b:c:d", long_options, &option_index)) != -1){
     switch(ch){
     case 'a': ;
       
       if(verbose)
	 fprintf(stdout, "--rdonly %s \n", optarg);
       int ifd;
       ifd = open(optarg, O_RDONLY);
       if(ifd >= 0){
	 	 if(fd_factor < fd_count - 1){
	   fd_factor = fd_factor + 5;
	   fd_array = (int*)realloc(fd_array, sizeof(int*)*(fd_factor));
	 }
	 fd_array[fd_count] = ifd;
	 fd_count++;
	 
	 }else{
	 fprintf(stderr, "Unable to open file %s. error = %d, message = %s \n", optarg, errno, strerror(errno));
	 fd_array[fd_count] = -1;
	 fd_count++;
	 error_no = 1;
	 // exit(1);
       }
       break;
     case 'b':
       if(verbose)
	 fprintf(stdout, "--wronly %s \n", optarg);
       ifd = open(optarg, O_WRONLY);
       if(ifd >= 0){
	 if(fd_factor < fd_count - 1){
	    fd_factor = fd_factor + 5;
	    fd_array = (int*)realloc(fd_array, sizeof(int*)*(fd_factor));
	 }
	 fd_array[fd_count] = ifd;
	 fd_count++;
	 
       }else{
	 fprintf(stderr, "Unable to open file %s. error = %d, message = %s \n", optarg, errno, strerror(errno));
	 error_no = 1;
	 fd_array[fd_count] = ifd;
	 fd_count++;
	 //exit(1);
       }
       break;
     case 'c':;
       if(verbose)
	 fprintf(stdout, "--command");
       
       optind--;
       int tempind = optind;
       int count = 0;
       char* str = "";
       //checking number of arguments
       while(optind < argc){
	 str = argv[optind];
         if(strncmp("--", str, 2) == 0){
           break;
	 }
	 if(verbose){
	   fprintf(stdout, " %s", str);
         }
	 optind++;
	 count++;	 
       }
       if(verbose)
	 fprintf(stdout, "\n");
       if(count<=3){
	 fprintf(stderr, "wrong number of arguments used for command. error = %d, message = %s \n", errno, strerror(errno));
	 exit(1);
       }
       optind = tempind;
       
       int i = atoi(argv[optind++]);
       int o = atoi(argv[optind++]);
       int e = atoi(argv[optind++]);
       
       if(i >= fd_count || o >= fd_count || e>=fd_count || i<0 || o<0 || e<0){
	  fprintf(stderr,"that many files have not been entered yet. error = %d, message = %s \n", errno, strerror(errno));
	  exit(1);
       }
       
       i = fd_array[i];
       o = fd_array[o];
       e = fd_array[e];
       char* cmd = argv[optind++];
       
       char** args = (char**)malloc(sizeof(char*)*(count+2));
       int argcount = 1;
       args[0] = cmd;
       while(argcount < count-3){
	 args[argcount] = argv[optind];
	 argcount++;
	 optind++;
       }
       
       args[argcount] = NULL;
       int child_pid = fork();
       if(child_pid < 0){
	 fprintf(stderr, "Unable to fork. error = %d, message = %s \n", errno, strerror(errno));
       }else if(child_pid == 0){
	 if(dup2(i, STDIN_FILENO) == -1)
	   fprintf(stderr, "Unable to duplicate input file descriptor. error = %d, message = %s \n", errno, strerror(errno));
	 if(dup2(o, STDOUT_FILENO) == -1)
	    fprintf(stderr, "Unable to duplicate output file descriptor. error = %d, message = %s \n", errno, strerror(errno));
	 if(dup2(e, STDERR_FILENO) == -1)
	   fprintf(stderr, "Unable to duplicate err file descriptor. error = %d message = %s \n", errno, strerror(errno));
	 execvp(*args, args);
	 close(i);
	 close(o);
	 close(e);
      	 exit(0);
       }
       else{
       }
       break;
     case 'd':
       verbose = 1;
       break; 
     default:
       fprintf(stderr, "Incorrect command: '%s' \n", optarg);
       error_no = 1;
       break;
     }
}
   exit(error_no);
}
		
