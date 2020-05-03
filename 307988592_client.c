 #include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<unistd.h>
#include<netdb.h>
#define def_http "HTTP/1.0"
#define  separated "\r\n"
#define no_path "/"
#define h_addr h_addr_list[0]
#define BUFLEN 256
void usage();
int find_string(int size, char* s[],char* search);
int check_format(char* s);
int find_url(char* arg_v);
char* find_host(char* url);
char* find_file_path(char* url);
int find_port(char* url);
void build_client_side(char* host,int port,char* request);
int main(int argc , char* argv[]){
    if(argc<2||argv==NULL){
        return -1;// cell Usage function
    }
    char* p_text;
    int find_R=0;
    int after_R =0;
    int size_r =0;
    int index=1;
    int get_or_post = 0;
    int Content_Length =0;
    int request_size=0;
    char* URL =NULL;
    int find_P=0;
    /*In this code snippet I go through the array
    while checking for exceptions with the user's
    address and revenue as well as counting 
    the sizes to assign the request array after*/
    while(index<argc){
        if(find_url(argv[index])==0){// if we find http:// so after we have the addreses
            URL = argv[index];//find so get
            index++;
        }   
        if(index==argc)
            break;  
        if(strcmp(argv[index],"-p")==0 && find_P==0){// if we have input string "body"
            if(index==argc-1){// the user said i will input string but is not
                usage();
                exit(0);
            }
            else{
                index++;
                p_text = argv[index];
                if(p_text!=NULL){
                    get_or_post = 1;
                    Content_Length = strlen(p_text);
                    request_size+=2*strlen(separated);
                    request_size += strlen(p_text);
                    request_size+=strlen(separated);
                    request_size+=strlen("Content-length:");
                    find_P=index;
                    index++;
                }
                else{// have no string after P so need to cell useage
                    usage();
                    exit(0);
                }
            }
        }
        if(index==argc)
            break;
        if(strcmp(argv[index],"-r")==0 && find_R ==0){// in put arg we check it 
            index++;
            size_r = atoi(argv[index]);// get the number from "num"
            if(size_r<0){
                usage();
                exit(0);
            } 
            request_size+=strlen("GET ");
            index++;
            find_R=index;
            int i;
            for(i=0;i<size_r;i++){// we pulover the arg and check tat num=to how arg he put
                if(i+index<argc){
                    if(size_r-after_R!=0 && check_format(argv[find_R+i])==0){
                        request_size += strlen(argv[find_R+i]);
                        after_R++;
                    }
                    else{
                            usage();
                            exit(0);
                    }
                }
                else{
                    break;
                }  
            }
            index=index+after_R;
            if(after_R!=size_r){
                usage();
                exit(0);
            }
        }
        if(after_R!=size_r){
                usage();
                exit(0);
        }
        if(index==argc)
            break;
        else{
            if( check_format(argv[index])==0){
                usage();
                exit(0);
            }
            if(size_r==0 && find_P==0){// 2*url, or string 
                usage();
                exit(0);
            }
            if(size_r==0 && find_P!=0 && find_url(argv[index])==-1){//none R and have P and is not the url
                usage();
                exit(0);
            }
            if(size_r!=after_R){
                usage();
                exit(0);
            }
        }
    }
    if(URL==NULL)
    {
        usage();
        exit(0);
    }
    if(argc!=index){
        usage();
        exit(0);
    }
    char* host =find_host(URL);//get host after  http
    char* file_path =find_file_path(URL);// filepath from url
    int port =find_port(URL);// get the port
    if(port<=0){
        usage();
        exit(0);
    }
    request_size+=strlen(host);
    request_size+=strlen(file_path);
    request_size+= strlen(" HTTP/1.0\r\n");
    request_size+=strlen("HOST: ");
    request_size+=strlen("Request:");
    request_size+=strlen(separated);
    char* request = (char*)calloc(request_size*2,sizeof(char));
    if(request==NULL){
        perror("malloc faild\n");
        exit(1);
    }
    memset(request,'\0',request_size*2);
    request[0]='\0';
    /*This code snippet builds the thread of the request or
     message in a standard way while chaining the strings*/
    if(get_or_post==0){// so  type request is get
        strcat(request,"GET ");
        strcat(request,file_path);
        if(size_r!=0){// have arguments in 
            strcat(request,"?");
            for(int i=0;i<size_r;i++){
                strcat(request,argv[find_R+i]);
                if(i<size_r-1)
                    strcat(request,"&");
            }
        }
        strcat(request," HTTP/1.0\r\n");
        strcat(request,"Host: ");
        strcat(request,host);
        strcat(request,separated);
        strcat(request,separated);
    }
    else{// get_or_post=1 -> request =post
        strcat(request,"POST ");
        strcat(request,file_path);
        if(size_r!=0){
            strcat(request,"?");
            for(int i=0;i<size_r;i++){
                strcat(request,argv[find_R+i]);
                if(i<size_r-1)
                    strcat(request,"&");
            }
        }
        strcat(request," HTTP/1.0\r\n");
        strcat(request,"Host: ");
        strcat(request,host);
        strcat(request,separated);
        if(Content_Length!=0){
            char str[12];
            strcat(request,"Content-length:");
            sprintf(str,"%d",Content_Length);
            strcat(request,str);
            strcat(request,separated);
            strcat(request,separated);
            strcat(request,p_text);
        }
        else{
            strcat(request,separated);
        }    
    }
    printf("Request:\n");
    printf("%s\n",request);
    build_client_side(host,port,request);
    free(host);
    free(request);
    return 0;
}

void build_client_side(char* host,int port,char* request){
    int Total = 0 ;
    struct sockaddr_in serv_addr; 
    struct hostent *server;
    server = gethostbyname(host);// get ip for address on server side
    if (server == NULL) { // have no addreses
        fprintf(stderr,"ERROR, no such host\n");
        free(host);
        free(request);
        exit(0);
    } 
    serv_addr.sin_family = AF_INET; //
    bcopy((char *)server->h_addr,
    (char *) &serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(port);
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd==-1){
        perror("socket filed\n");
        free(host);
        free(request);
        exit(0);
    }
    
    int rc = connect(sockfd, (const struct sockaddr*) &serv_addr, sizeof(serv_addr));
    if(rc<0){
        perror("error conencting\n");
        free(host);
        free(request);
        exit(0);
    }
    
    rc = write(sockfd,request,strlen(request)+1);
    if(rc<0){
        perror("write  is faild");
        free(host);
        free(request);
        exit(1);
    }
    
    char rbuf[BUFLEN];
    while(1){
        memset(rbuf,'\0',BUFLEN);
        rc =read(sockfd,rbuf,BUFLEN-1);
        Total+=rc;
        if(rc==0)
            break;
        printf("%s\n",rbuf);
    }
    
    printf("\n Total received response bytes:%d\n",Total);
    close(sockfd);
    
}
int find_url(char* arg_v){
    char http[7] ="http://";
    int j=0;
    int i;
    for(i=0;i<7;i++){
        if(arg_v[i]==http[i])
            j++;
    }
    if(j==7){
        j=0;
    }
    else{
        j=-1;
    }
    return j;
    
}
char* find_host(char* url){

    char* p= url;
    p=p+7;//point to after http://
    char tmp[strlen(p)];
    char* Q =strchr(p,':');
    int i;
    /*have two state if have port or def */
    if(Q==NULL){
        Q=strchr(p,'/');
    }
    int j=0;
    for(i=0;i<strlen(p);i++){
        if(Q==NULL){
            tmp[i]=p[i];
            j++;
        }
        else{
            if(p[i]==Q[0])
                break;
            tmp[i]=p[i];
            j++;
        }
    }
    char * host=(char*)calloc(j+1,sizeof(char));
    memset(host,'\0',j+1);
    strncpy(host,tmp,j);
    return host;
}
char* find_file_path(char* url){
    char*p = url;
    p=p+7;
    char * file_path = strchr(p,'/');
    if(file_path==NULL)
        file_path = "/";
    return file_path;
}
int find_port(char* url){
    char * p  = url;
    p=p+7;
    int port=0;
    char * tmp_port= strchr(p,':');
    if(tmp_port==NULL){//user dont input port
        port=80;
    }
    else{
        tmp_port =tmp_port+1;
        char t[strlen(tmp_port)];
        char* Q = strchr(tmp_port,'/');
        if(Q!=NULL){
            int i;
            for(i=0;i<strlen(t);i++){
                if(tmp_port[i]==Q[0])
                    break;
                t[i]=tmp_port[i];
            }
            t[i]='\0';
        } 
        port=atoi(tmp_port);
    }
    return port;
}
void usage(){
    printf("client [-p <text>] [-r n < pr1=value1 pr2=value2 …>] <URL>\n");
}
/*this function is only for my checking not use for the build*/
int find_string(int size,char*s[],char* search){
    char * match;
    int i =0;
    match= strstr(s[i],search);
    while(match==NULL){
        match= strstr(s[i],search);
        i++;
        if(i==size){
            break;
        }
    }
    if(match==NULL)
        return -1;
    return i;
}
int check_format(char* s){
    int fleg_format = -1;
    char* p= s;
    p++;
    if(strlen(p)<2)
        return -1;
    for(int i=0;i<strlen(p)-1;i++){//check tat ave som...=som..
        if(s[i]=='=')
            fleg_format = 0;
    }
    return fleg_format;
}