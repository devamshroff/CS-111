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
#include <sys/wait.h>


//flag variables
int errno;
int bit_flag = 0;
int* fd_array;
int fd_count;
int wait_flag = 0;
int wait_index = -1;
char** cmd_array;
int* id_array;
char* cmd_list;
int cmd_count = 0;
int exited_with_signal = 0;
int sig_term = 0;


void setflag(char* flag){
  if(strcmp(flag, "--append") == 0)
    bit_flag |= O_APPEND;
  else if(strcmp(flag, "--cloexec") == 0)
    bit_flag |= O_CLOEXEC;
  else if(strcmp(flag, "--creat") == 0)
    bit_flag |= O_CREAT;
  else if(strcmp(flag, "--directory") == 0)
    bit_flag |= O_DIRECTORY;
  else if(strcmp(flag, "--dsync") == 0)
    bit_flag |= O_DSYNC;
  else if(strcmp(flag, "--excl") == 0)
    bit_flag |= O_EXCL;
  else if(strcmp(flag, "--nofollow") == 0)
    bit_flag |= O_NOFOLLOW;
  else if(strcmp(flag, "--nonblock") == 0)
    bit_flag |= O_NONBLOCK;
  else if(strcmp(flag, "--rsync") == 0)
    bit_flag |= O_RSYNC;
  else if(strcmp(flag, "--sync") == 0)
    bit_flag |= O_SYNC;
  else if(strcmp(flag, "--trunc") == 0)
    bit_flag |= O_TRUNC;
  return;
  }

void close_fdarray(){
  for(int i = 0; i < fd_count; i++){
    close(fd_array[i]);
    fd_array[i] = -1;
  }
}

void seg_handler(int num){
    fprintf(stderr, "%d caught \n", num);
    fflush(stdout);
    exit(num);
}

//lab1b - everything except profile
int main(int argc, char** argv){
    int ch;
    int fd_factor = 10;
    fd_array = (int*)malloc(sizeof(int*)*fd_factor);
    int cmd_factor = 50;
    cmd_array = (char**)malloc(sizeof(char**)*cmd_factor);
    // cmd_list = (char*)malloc(sizeof(char*)*cmd_factor); 
    id_array = (int*)malloc(sizeof(int*)*cmd_factor);
    //cmd_names = (char*)malloc(sizeof(char*)*cmd_factor);
    
    int verbose = 0;
    int pipe_fd[2];
    static struct option long_options[] =
    {
       //lab1a options
	{"rdonly", required_argument, 0, 'a'},
        {"wronly", required_argument, 0, 'b'},
	{"command", required_argument, 0, 'c'},
	{"verbose", no_argument, 0, 'd'},

	//lab1b options
	//file open time flags
     
        {"append",      no_argument, 0,   'e'},
	{"cloexec",     no_argument, 0,   'f'},
	{"creat",       no_argument, 0,   'g'},
	{"directory",   no_argument, 0,   'h'},
	{"dsync",       no_argument, 0,   'i'},
        {"excl",        no_argument, 0,   'j'},
	{"nofollow",    no_argument, 0,   'k'},
	{"nonblock",    no_argument, 0,   'l'},
	{"rsync",       no_argument, 0,   'm'},
	{"sync",        no_argument, 0,   'n'},
	{"trunc",       no_argument, 0,   'o'},

	//file opening options
	{"rdwr", required_argument, 0, 'p'},
	{"pipe", no_argument, 0, 'q'},

	//miscellaneous
	{"close", required_argument, 0, 'r'},
	{"profile", no_argument, 0, 's'},
	{"abort", no_argument, 0, 't'},
	{"catch", required_argument, 0, 'u'},
	{"ignore", required_argument, 0, 'v'},
	{"default", required_argument, 0, 'w'},
	{"pause", no_argument, 0, 'x'},     
	{"wait", no_argument, 0, 'y'},
	
	{0,0,0,0}
	
    };
    
    int error_no = 0;
    int option_index = 0;
    while((ch = getopt_long(argc, argv, "a:b:c:defghijklmnop:qr:stu:v:w:xy", long_options, &option_index)) != -1){
      switch(ch){
      case 'a': ;
	if(verbose)
	  fprintf(stdout, "--rdonly %s \n", optarg);
	int ifd;
	ifd = open(optarg, O_RDONLY | bit_flag, 0644);
	bit_flag = 0;
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
	}
	break;
      case 'b':
	if(verbose)
	  fprintf(stdout, "--wronly %s \n", optarg);
	ifd = open(optarg, O_WRONLY | bit_flag, 0644);
	bit_flag = 0;
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
	}
	fflush(stdout);
	break;
	
      case 'p': ;
	if(verbose)
	  fprintf(stdout, "--rdwr %s \n", optarg);
	ifd = open(optarg, O_RDWR | bit_flag, 0644);
	bit_flag = 0;
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
       }
	fflush(stdout);
	break;
	
      case 'q':
	if(verbose)
	 fprintf(stdout, "--pipe \n");
	if(pipe(pipe_fd) < 0)
	  fprintf(stdout, "unable to create pipe. error: %d, message: %s", errno, strerror(errno));
	fd_array[fd_count] = pipe_fd[0]; //for the read
	fd_count++;
	fd_array[fd_count] = pipe_fd[1]; //for the write
	fd_count++;
	fflush(stdout);
	break;
	
      case 'c': ;
	char* cmd_list = (char*)malloc(sizeof(char*)*cmd_factor); 
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
	 if(count >= 3){
	   cmd_list = strcat(cmd_list, str);
	   cmd_list = strcat(cmd_list, " ");
	 }
	 optind++;
	 count++;	 
       }
       
       if(verbose)
	 fprintf(stdout, "\n");
       if(count<=3){
	 fprintf(stderr, "wrong number of arguments used for command. error = %d, message = %s \n", 1, strerror(1));
	 error_no = 1;
	 fflush(stdout);
	 continue;
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

       if(i == -1 || o == -1 || e == -1)
	 error_no = 1;
       
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
       fflush(stdout);
       int child_pid = fork();
       if(child_pid < 0){
	 fprintf(stderr, "Unable to fork. error = %d, message = %s \n", errno, strerror(errno));
	 error_no = 1;
       }else if(child_pid == 0){ //child process
	 if(dup2(i, STDIN_FILENO) == -1){
	   fprintf(stderr, "Unable to duplicate input file descriptor. error = %d, message = %s \n", errno, strerror(errno));
	   fflush(stdout);
	   error_no = 1;
	 }
	 if( dup2(o, STDOUT_FILENO) == -1){
	   fprintf(stderr, "Unable to duplicate output file descriptor. error = %d, message = %s \n", errno, strerror(errno));
	   fflush(stdout);
	   error_no = 1;
	 }
	 if(dup2(e, STDERR_FILENO) == -1){
	    fprintf(stderr, "Unable to duplicate err file descriptor. error = %d message = %s \n", errno, strerror(errno));
	    fflush(stdout);
	    error_no = 1;
	 }
	 close_fdarray();
	 if(execvp(*args, args) == -1){
	   fprintf(stderr, "An error has occured in execvp. error: %d, message: %s \n", errno, strerror(errno));
	   fflush(stdout);
	   error_no = 1;
	 } 
	 close(i);
	 close(o);
	 close(e);
      	 exit(0);
	 fflush(stdout);
       }
       else{ //parent process

	 if(cmd_count >= cmd_factor-1){
	   cmd_factor = cmd_factor*2;
	   id_array = realloc(id_array, sizeof(int*)*(cmd_factor));
	   cmd_array = realloc(cmd_array, sizeof(char**)*(cmd_factor));
	 }
	 
	 id_array[cmd_count] = child_pid;
	 cmd_array[cmd_count++] = cmd_list;
	 cmd_list = "";
	 fflush(stdout);
       }
       break;
     case 'd':
       if(verbose)
	 fprintf(stdout, "--verbose \n");
       fflush(stdout);
       verbose = 1;
       break;

     case 'e': case 'f': case 'g': case 'h': case 'i': case 'j': case 'k':
     case 'l': case 'm': case 'n': case 'o':
       if(verbose)
	 fprintf(stdout, "%s \n", argv[optind-1]);
       char* flag = argv[optind - 1];
       setflag(flag);
       break;
            
      case 'r': //close
	if(verbose)
	  printf("--close %d \n", atoi(optarg));
	fflush(stdout);
	close(fd_array[atoi(optarg)]);
	fd_array[atoi(optarg)] = -1;
	break;
	
      case 's': //profile
	if(verbose)
	  printf("--profile \n");
	break;
	
      case 't': //abort
	if(verbose)
	  printf("--abort \n");
	fflush(stdout);	
	if(raise(SIGSEGV) != 0)
	  fprintf(stderr, "Could not abort. error: %d, message: %s \n", errno, strerror(errno));
       break;
       
      case 'u': //catch
	if(verbose)
	  printf("--catch %d \n", atoi(optarg));
	fflush(stdin);
	if(signal(atoi(optarg), seg_handler) == SIG_ERR)
	   fprintf(stderr, "unable to set segfault handler for catch. error: %d, message: %s \n", errno, strerror(errno));
	break;
	
      case 'v': //ignore
	if(verbose)
	  printf("--ignore %d \n", atoi(optarg));
	fflush(stdout);	
	if(signal(atoi(optarg), SIG_IGN) == SIG_ERR)
	  fprintf(stderr, "unable to ignore. error: %d, message: %s \n", errno, strerror(errno));
	break;
	
      case 'w': //default;
	if(verbose)
	  fprintf(stdout, "--default %d \n", atoi(optarg));    
	fflush(stdout);
	if(signal(atoi(optarg), SIG_DFL) == SIG_ERR)
	   fprintf(stderr, "unable to do the default action. error: %d, message: %s \n", errno, strerror(errno));
	break;

	
      case 'x': //pause
	if(verbose)
	  printf("--pause \n");
	pause();
	break;

      case 'y': //wait
	
	if(verbose)
	  printf("--wait \n");
	wait_flag = 1;
	//need a flag for if process exited becasue of a signal
	wait_index = 0;
	int wstatus = 0;
	int cpid;
	while(cmd_count){
	  cpid = wait(&wstatus);

	  int i;

	  for (i = 0; i < cmd_count; i++) {
	    if (cpid == id_array[i]) {
	      break;
	    }
	  }

	  if(WIFEXITED(wstatus)){
	    int exit_status = WEXITSTATUS(wstatus);
	    error_no = exit_status > error_no ? exit_status : error_no;
	    fprintf(stdout, "exit %d %s\n", exit_status, cmd_array[i]);
	    fflush(stdout);
          } else {
            int sig_temp_status = WTERMSIG(wstatus);
	    sig_term = sig_temp_status > sig_term ? sig_temp_status: sig_term;
	    exited_with_signal = 1;
	    fprintf(stdout, "signal %d %s\n", sig_temp_status, cmd_array[i]);
	    fflush(stdout);
	  }

	  cmd_array[i] = NULL; 
	  // id_array[i] = NULL;
	  for(int j = i ; j < cmd_count; j++){
	    cmd_array[j] = cmd_array[j+1];
	    id_array[j] = id_array[j+1];
	  }
	  cmd_count--;
	}
	break;	
      default:
	fprintf(stderr, "Incorrect command: '%s' \n", optarg);
	error_no = 1;
	break;
      }
    }
    fflush(stdout);

    if(exited_with_signal){
      signal(sig_term, SIG_DFL);
      raise(sig_term);
    }
    exit(error_no);
}
