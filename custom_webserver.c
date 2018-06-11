#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libgen.h>
#include <signal.h>
#include <sys/wait.h>          //Added 7-5-2017
#include <sys/sendfile.h>      //Added 7-5-2017

#define PROT_HTTP11 "HTTP/1.1"

#define STAT_200 " 200 OK\r\n"
#define STAT_404 " 404 Not Found\r\n"
#define STAT_501 " 501 Not Implemented\r\n"

#define F_DIR "Content-Type: text/directory\r\n"
#define F_GIF "Content-Type: image/gif\r\n"
#define F_HTML "Content-Type: text/html\r\n"
#define F_ICO "Content-Type: image/x-icon\r\n"
#define F_JPEG "Content-Type: image/jpeg\r\n"
#define F_JPG "Content-Type: image/jpg\r\n"
#define F_TXT "Content-Type: text/plain\r\n"

typedef enum {cgi, gif, html, ico, jpeg, jpg, plain, noext} ext;

/* Function that returns a string with the name of the files extension. */
ext get_ext(char *file) {
    if (strstr(file, ".cgi") != NULL)
        return cgi;
    if (strstr(file, ".gif") != NULL)
        return gif;
    if (strstr(file, ".html") != NULL)
        return html;
    if (strstr(file, ".ico") != NULL)
    return ico;
    if (strstr(file, ".jpeg") != NULL)
        return jpeg;
    if (strstr(file, ".jpg") != NULL)
        return jpg;
    if (strstr(file, ".txt") != NULL)
        return plain;
    return noext; /* There is no extension. */
}


char webpage[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: text/html; charset=UTF-8\r\n\r\n"			/*			*/
"<!doctype html>\r\n"
"<html><head><title>B2badmin</title>\r\n"				/*	HTML CODE	*/
"<style>body {background-color: #FFF00 }</style></head>\r\n"
"<body><center><h1>Hello world!</h1><br>\r\n"				/*			*/
"<img src=\"test.jpg\"></center></body></html>\r\n";

char jpgheader[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: image/jpeg\r\n\r\n";

char icoheader[] =
"HTTP/1.1 200 OK\r\n"
"Content-Type: image/x-icon\r\n\r\n";

char * get_url(char *);
char * get_fname(char *);
long long get_f_size(char *);
void DieWithError(char *errorMessage);
int CreateTCPServerSocket (unsigned short port); 
void SendToClientSocket(int clntSock, char *buf);

int main()
{    	
    printf("hi");										
    struct sockaddr_in client_addr; /* ------------------------------------> Clients IP-Socket. */	 
    socklen_t sin_len = sizeof(client_addr); /* ---------------------------> We always pass into function the size value so it can be updated. */				
    int servSock , clntSock; /* -------------------------------------------> Servers Socket, Clients Socket. */
    char buf[2048];
    pid_t processID;
    int status;
    unsigned int childProcCount = 0;
     //Added 7-5-2017
    servSock = CreateTCPServerSocket(58080);
    
    
    while(1)
    { 
	/* -The accept function is called by a TCP server to return				  */
	/* the first connection request on the queue of pending                                   */
	/* connections for the listening socket 		  				  */
	
	/* -This call returns a non-negative descriptor on success,				  */			
	/* otherwise it returns -1 on error. 							  */

	/* -The returned descriptor is assumed to be a client socket descriptor and all		  */
	/* read-write operations will be done on this descriptor to communicate with the client.  */	
	/* =====================================================================================  */
	/* int accept (int sockfd, struct sockaddr *cliaddr, socklen_t *addrlen);                 */
	/* sockfd − It is a socket descriptor returned by the socket function.  		  */
	/* cliaddr − It is a pointer to struct sockaddr that contains client IP address and port. */
	/* addrlen − Set it to sizeof(struct sockaddr).                                           */
        clntSock = accept(servSock, (struct sockaddr *) &client_addr, &sin_len);
        if(clntSock == -1)
        {
            perror("Connection failed...\n");
            continue;
        }

        printf("Got client connection...\n");
	/* fork() creates a child process that differs from the parent process only in its PID       */
	/* and PPID, and in the fact that resource utilizations are set to 0.			     */	
	/* File locks and pending signals are not inherited.					     */
	/* ===================================================================================       */
	/* On success, the PID of the child process is returned in the parent’s thread of execution, */
 	/* and a 0 is returned in the child’s thread of execution.				     */
 	/* On failure, a -1 will be returned in the parent’s context,                                */
	/* no child process will be created, and errno will be set appropriately.                    */
        if ((processID = fork()) < 0)
        {
            printf("fork() failed");
        }
        else if ( processID == 0 ) {
            /* in child */

	    /* The close function is used to close the communication between */
	    /* the client and the server.                                    */
	    /* ============================================================  */
	    /* int close( int sockfd ); */
             close(servSock);
 
	    /* The C library function void *memset(void *str, int c, size_t n) copies the         */
	    /* character c (an unsigned char) to the first n characters of the string pointed to, */
            /* by the arqument str.                                                               */
	    /* =================================================================================  */
	    /* void *memset(void *str, int c, size_t n) 					  */
	    /* str − This is a pointer to the block of memory to fill.				  */
	    /* c − This is the value to be set. The value is passed as an int, but the function   */
            /* fills the block of memory using the unsigned char conversion of this value.        */
	    /* n − This is the number of bytes to be set to the value.                            */	
            memset(buf, 0, 2048); /* --> buf[] = { '0', '0', '0' . . . } */

	    /* The read function attempts to read nbyte bytes from the file            */
	    /* associated with the buffer, fildes, into the buffer pointed to by buf.  */
            /* =====================================================================   */
            /* int read(int fildes, const void *buf, int nbyte);                       */
            /* fildes − It is a socket descriptor returned by the socket function.     */
            /* buf − It is the buffer to read the information into.                    */
	    /* nbyte − It is the number of bytes to read.                              */
            read(clntSock, buf, 2047);
            SendToClientSocket(clntSock, buf);
            printf("closing...\n");
            close(clntSock);
            exit(0);
        }
        else {
            /* in parent */
            
            // printf ("In parent \n");
            // if (waitpid(child, NULL, 0) < 0) {
            //     perror("Failed to collect child process");
            //     break;
            // }


            printf("with child process' %d\n", (int) processID);
            close(clntSock);
            /* Parent closes child socket descriptor */
            childProcCount++;
            /* Increment number of outstanding child processes */
            while (childProcCount) /* Clean up all zombies */
            {
                processID = waitpid((pid_t) -1, NULL, WNOHANG); /* Nonblocking wait */
                if (processID < 0) /* waitpid() error? */
                    DieWithError("waitpid() failed");
                else if (processID == 0) /* No zombie to wait on */
                    break;
                else
                    childProcCount--; /* Cleaned up after a child */
            }
            
        }
        /* parent process */
        close(clntSock);

    }
    return 0;
}

char * get_url(char * buf)
{
    char *p1 = strstr(buf, " ")+1;
    char *p2 = strstr(p1, " ");
    int len = p2-p1;
    char *url;
    url = (char *) malloc(2048);
    memset(url, '\0', 2048);
    //printf("%d\n",len);
    strncpy(url, p1, len);
    //printf("%s\n",url);
    return url;
}

char * get_fname(char * buf)
{
    return basename(strdup(get_url(buf)));
}

long long get_f_size(char * buf)
{
    long long size;
    struct stat st;
    stat(get_fname(buf), &st);
    size = st.st_size;
    printf("The size of file is : %lli\n",size);
    return size;
}

void DieWithError(char *errorMessage)
{
    /* Prints the corresponding error. */
    perror ( errorMessage) ;
    exit(1);
}

int CreateTCPServerSocket (unsigned short port)
{
    int servSock;
    int on = 1;
    struct sockaddr_in server_addr;
    /* Family: AF_INET | Type: SOCK_STREAM | PROTOCOL: Systems default  */
    /* ================================================================ */
    /* -->socket() creates an endpoint for communication and returns a  */
    /* file descriptor that refers to that endpoint.                    */
    servSock = socket(AF_INET, SOCK_STREAM, 0);
  
    /* In case servSock < 0 ,it pops up an error. */
    if(servSock < 0)
    {
        perror("socket");
        exit(1);
    }
    /* Socket descriptor: servSock | Level protocol: socket level(SOL_SOCKET) | OPTIONAME: SO_REUSEADDR(enable/disable local address reuse) | */
    /* const void *optval | socklen_t option_len 									                      */
    setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(int));

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;     /* INADDR_ANY value for IP address means the server's IP address will be assigned automatically. */
    server_addr.sin_port =  htons(port); /* This function converts 16-bit (2-byte) quantities from host byte order to network byte order. */
 
    /* Sockfd: servSock | *my_addr: points to server_addr | addrlen: sizeof(server_addr) */
    /* ================================================================================= */
    /* When a socket is created with socket(2), it exists in a name space                */
    /* (address family) but has no address assigned to it.  bind() assigns               */
    /* the address specified by addr to the socket referred to by the file               */
    /* descriptor sockfd.  addrlen specifies the size, in bytes, of the                  */
    /* address structure pointed to by addr.  Traditionally, this operation              */
    /* is called “assigning a name to a socket”.                                         */
    /**/
    /* This call returns 0 if it successfully binds to the address, otherwise it returns -1 on error. */		
    if(bind(servSock, (struct sockaddr *) &server_addr, sizeof(server_addr)) == -1)
    {
        perror("bind");
        close(servSock);
        exit(1);
    }
    /* The listen function converts an unconnected socket into a passive socket,     */
    /* indicating that the kernel should accept incoming connection                  */
    /* requests directed to this socket.                                             */	
    /* ==========================================================================    */
    /* int listen(int sockfd,int backlog)                                            */
    /* sockfd − It is a socket descriptor returned by the socket function.           */	
    /* backlog − It is the number of allowed connections.                            */
    /* This call returns 0 on success, otherwise it returns -1 on error.             */
    if(listen(servSock, 10) ==-1)
    {
        perror("listen");
        close(servSock);
        exit(1);
    }
    return servSock;
}

void SendToClientSocket(int clntSock, char *buf)
{
    char *url, *f_name;
    ext f_ext;
    long long f_size;
    int fdimg;
    int retsendfile;

    url = get_url(buf);
    f_name = get_fname(buf);
    f_ext = get_ext(f_name);
    printf("%s\n", buf);
    if(f_ext != noext)
    { 
        f_size = get_f_size(buf);
        fdimg = open(f_name, O_RDONLY); 
        if(fdimg >0 )
        {
            if (f_ext == jpg) {
                write(clntSock, jpgheader, sizeof(jpgheader) - 1);
            }
            else if (f_ext == ico) {
                write(clntSock, icoheader, sizeof(icoheader) - 1);
            }
            retsendfile = sendfile(clntSock, fdimg, NULL, f_size);
            if(retsendfile==-1)
            {
                perror("send file \n");
            }
            else
            {
                printf("%d bytes send to client for file %s \n", retsendfile,f_name);
            }
            close(fdimg);
        }
        else
        {
            perror("File not found");
            write(clntSock, webpage, sizeof(webpage) - 1);
        }
        
    }
    else
    {
        write(clntSock, webpage, sizeof(webpage) - 1);
    }
    return;
}
