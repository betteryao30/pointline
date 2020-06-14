#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>

#define FILE_NAME_MAX_SIZE 512
#define BUFFER_SIZE 1024

void sendrecvfile(int clientId);  //发送文件名获取服务器上的该文件并接收
void sendfile(int clientId);      //发送文件给服务器
void recvfile(int clientId);
void chat(int clientId,int ret);  //发送聊天文字信息
void recard(char *message);  //记录信息

int main()
{
    //定义Internet协议结构,客户端的IP信息
     struct sockaddr_in myaddr;
     memset(&myaddr,0,sizeof(myaddr));
     myaddr.sin_family = PF_INET;
     myaddr.sin_port = htons(1314);
     myaddr.sin_addr.s_addr = inet_addr("175.24.78.187");

  //1.创建套接字
   int clientId = socket(PF_INET,SOCK_STREAM,0);
   if(clientId<0)
   {
    perror("socket\n");
    return  -1;
   }
   printf("socket ok\n");
  //2发起链接请求
  int ret = connect(clientId,(struct sockaddr *)&myaddr,sizeof(myaddr));
  
  //输入连接密码
    char password[FILE_NAME_MAX_SIZE+1];    
    bzero(password, FILE_NAME_MAX_SIZE+1); 
    char buf[BUFFER_SIZE];    
    bzero(buf, BUFFER_SIZE);
    //输入连接码
    printf("请输入连接码:\t");    
    scanf("%s", password);
    strncpy(buf, password, strlen(password)>BUFFER_SIZE?BUFFER_SIZE:strlen(password));        
     // 向服务器发送buffer中的数据    
    if(send(clientId, buf, BUFFER_SIZE, 0) < 0)    
    {        
        perror("Check Failed:");        
        exit(1);    
    }

  if(ret <0)
  {
    perror("connect\n");
    close(clientId);
    return -1;
  }
  printf("connect ok\n");
  
  /*
  // 3接收消息
  */
  chat(clientId,ret);
  
  //收发文件
  //sendrecvfile(clientId);

  //只收文件
  //recvfile(clientId,ret);

  //只发文件
  //sendfile(clientId);
   

  //4.关闭套接字
  close(clientId);
  return 0;
}

void sendfile(int clientId)  //发送文件
{
    char file_name[FILE_NAME_MAX_SIZE+1];    
    bzero(file_name, FILE_NAME_MAX_SIZE+1); 
    char buf[BUFFER_SIZE];    
    bzero(buf, BUFFER_SIZE);
    //输入待发送的文件名
    printf("请输入待发送的文件名:\t");    
    scanf("%s", file_name);
    strncpy(buf, file_name, strlen(file_name)>BUFFER_SIZE?BUFFER_SIZE:strlen(file_name));        
     // 向服务器发送buffer中的数据    
    if(send(clientId, buf, BUFFER_SIZE, 0) < 0)    
    {        
        perror("Send File Name Failed:");        
        exit(1);    
    }

    // 打开文件并读取文件数据
    //strncpy(buffer, file_name, strlen(file_name)>BUFFER_SIZE?BUFFER_SIZE:strlen(file_name));        
    FILE *fp = fopen(file_name, "r");        
    if(NULL == fp)        
    {            
        printf("File:%s Not Found\n", file_name);        
    }        
    else        
    {            
        bzero(buf, BUFFER_SIZE);      //将buf清零      
        int length = 0;            
        // 每读取一段数据，便将其发送给服务器，循环直到文件读完为止            
        while((length = fread(buf, sizeof(char), BUFFER_SIZE, fp)) > 0)            
        {              
            if(send(clientId, buf, length, 0) < 0)                
            {                   
                printf("Send File:%s Failed./n", file_name);                    
                break;                
            }                
            bzero(buf, BUFFER_SIZE);     //将buf清零      
        }                         
        printf("File:%s Transfer Successful!\n", file_name);        
    }
    // 关闭文件            
    fclose(fp);
}

void recvfile(int clientId)  //接收文件
{
    int ret = 0;
    // 输入文件名 并放到缓冲区buffer中等待发送 
    char buffer[BUFFER_SIZE];    
    bzero(buffer, BUFFER_SIZE);  
    ret = recv(clientId,buffer,sizeof(buffer),0);
    if(ret<0)
    {
        perror("recv\n");
        close(clientId);
        return -1;
    }
    // 然后从buffer(缓冲区)拷贝到file_name中        
    char file_name[FILE_NAME_MAX_SIZE+1];        
    bzero(file_name, FILE_NAME_MAX_SIZE+1);         //将filename清零
    strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buffer));
    bzero(buffer, BUFFER_SIZE); 
    printf("文件名为：\n");
    sleep(5);
    printf("%s\n", file_name); 
    printf("打开文件准备写入\n");
    

    // 打开文件，准备写入    
    FILE *fp = fopen(file_name, "w");    
    printf("打开文件成功\n");
    if(NULL == fp)    
    {        
        printf("File:\t%s Can Not Open To Write\n", file_name);        
        exit(1);    
    }     
    else
    {
        printf("开始写文件\n");
        // 从服务器接收数据到buffer中    
        // 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止    
        bzero(buffer, BUFFER_SIZE); 
        printf("清空buffer");   
        int length = 0; 
        printf("准备进入循环");   
        while((length = recv(clientId, buffer, BUFFER_SIZE, 0)) > 0)    
        {        
            printf("进入循环写文件");
            if(fwrite(buffer, sizeof(char), length, fp) < length)        
            {            
                printf("File:\t%s Write Failed\n", file_name);            
                break;        
            }        
            bzero(buffer, BUFFER_SIZE);    
        }        
        printf("Receive File:\t%s save Successful!\n", file_name);
    }
    printf("关闭文件\n");    
    close(fp); //关闭文件
}


void sendrecvfile(int clientId)  //发送文件名给服务器让服务器把该文件发过来
{
    while(1)
  {     
     // 输入文件名 并放到缓冲区buffer中等待发送    
     char file_name[FILE_NAME_MAX_SIZE+1];    
     bzero(file_name, FILE_NAME_MAX_SIZE+1);    
     printf("Please Input File Name On Server:\t");    
     scanf("%s", file_name);     
     char buffer[BUFFER_SIZE];    
     bzero(buffer, BUFFER_SIZE);    
     strncpy(buffer, file_name, strlen(file_name)>BUFFER_SIZE?BUFFER_SIZE:strlen(file_name));        
     // 向服务器发送buffer中的数据    
     if(send(clientId, buffer, BUFFER_SIZE, 0) < 0)    
     {        
         perror("Send File Name Failed:");        
         exit(1);    
    }  

    // 打开文件，准备写入    
    FILE *fp = fopen(file_name, "w");    
    if(NULL == fp)    
    {        
        printf("File:\t%s Can Not Open To Write\n", file_name);        
        exit(1);    
    }     
    else
    {
        // 从服务器接收数据到buffer中    
        // 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止    
        bzero(buffer, BUFFER_SIZE);    
        int length = 0;    
        while((length = recv(clientId, buffer, BUFFER_SIZE, 0)) > 0)    
        {        
            if(fwrite(buffer, sizeof(char), length, fp) < length)        
            {            
                printf("File:\t%s Write Failed\n", file_name);            
                break;        
            }        
            bzero(buffer, BUFFER_SIZE);    
        }        
        printf("Receive File:\t%s From Server IP Successful!\n", file_name);
    }    
    close(fp); //关闭文件

  }
}

void chat(int clientId,int ret)
{
    while(1)
    {
        char buf[BUFFER_SIZE];
        memset(buf,0,BUFFER_SIZE);
        ret = recv(clientId,buf,sizeof(buf),0);
        if(ret<0)
        {
            perror("recv\n");
            close(clientId);
            return -1;
        }
        if(strcmp(buf,"send")==0)
        {
            recvfile(clientId);
        }
        printf("received from xldserver:%s\n",buf);

        memset(buf,0,BUFFER_SIZE);
        bzero(buf, BUFFER_SIZE);
        printf("xldclient: ");
        gets(buf);
        if(strcmp(buf,"quit")==0)
        {
            break;
        }
        ret=send(clientId,buf,sizeof(buf),0);
        if(ret<0)
        {
            perror("send\n");
            close(clientId);
            return -1;
        }
        if(strcmp(buf,"send")==0)
        {
            sendfile(clientId);
        }
    }
}
