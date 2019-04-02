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

int errno;

void seg_handler(int num) {
  if(num == SIGSEGV){
    fprintf(stderr, "Segmentation fault caught! error = %d, message = %s \n", errno, strerror(errno));
    exit(4);
  }
}

void forceseg();

int main(int argc, char** argv){

  int ch;

  static struct option long_options[] =
    {
     {"input", required_argument, 0, 'i'},
     {"output", required_argument, 0, 'o'},
     {"segfault", no_argument, 0, 's'},
     {"catch", no_argument, 0, 'c'},
     {"dump-core", no_argument, 0, 'd'},
     {0,0,0,0}
    };

  int option_index = 0;
  
  while((ch = getopt_long(argc, argv, "i:o:scd", long_options, &option_index)) != -1){
    if(ch != 'i' && ch != 'o' && ch != 's' && ch != 's' && ch != 'c' && ch != 'd'){
      fprintf(stderr, "Invalid Argument. Proper Usage: --input='infile' --output='outfile'.");
      exit(1);
      
      }

    }
  option_index = 0;
  optind = 1;
  while((ch = getopt_long(argc, argv, "i:o:scd", long_options, &option_index)) != -1){
 
    switch(ch){

    case 'i': //input
      ;
      int ifd;
      ifd = open(optarg, O_RDONLY);
      if(ifd >= 0){
	close(0);
	dup(ifd);
	close(ifd);
      }else{
	fprintf(stderr, "Unable to open file %s. error = %d, message = %s \n", optarg, errno, strerror(errno));
	exit(2);
      }
      break;
      
    case 'o':
      ;
      int ofd;
      ofd = creat(optarg, 0666);
      if(ofd >= 0){
	close(1);
	dup(ofd);
	close(ofd);
      }else{
	fprintf(stderr, "unable to create / truncate %s. error = %d, message = %s \n", optarg, errno, strerror(errno));
	exit(2);
      }
      break;
      
    case 's':
      forceseg();
      break;
      
    case 'c':
      ;
      signal(SIGSEGV, seg_handler);      
      break;
      
    case 'd':
      signal(SIGSEGV, SIG_DFL);
      break;
      
    default:
      //fprintf(stderr, "incorrect arguments selected \n");
      fprintf(stderr, "Incorrect Argument. Proper usage: lab0 --input=filename --output=filename. Error: %d, Message: %s \n", errno, strerror(errno));
      exit(1);
      break;
    } 
  }
  char inp;
  while(read(0, &inp, sizeof(char)) > 0){
    if(write(1, &inp, sizeof(char)) < 0){
      fprintf(stderr, "Unable to write to output file. error = %d, message = %s \n", errno, strerror(errno));
      exit(3);
    }
  }
  exit(0);
}

void forceseg(){
  char* x = NULL;
  *x = 0;
}


