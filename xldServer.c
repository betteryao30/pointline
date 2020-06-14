#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 1024
#define FILE_NAME_MAX_SIZE 512

void sendrecvfile(int conId,int serverId,int ret);//发送文件名获取服务器上的该文件并接收
void sendfile(int clientId);      //发送文件给客户端
void recvfile(int clientId,int serverId,int ret);  //接收来自客户端的文件
void chat(int conId,int serverId,int ret);  //聊天
void recard(char *from,char *message);  //记录聊天信息

int main()
{
    //定义Internet协议结构,服务器的端口号和IP地址
     struct sockaddr_in myaddr;
     memset(&myaddr,0,sizeof(myaddr));
     myaddr.sin_family = PF_INET;
     myaddr.sin_port = htons(1314);
     myaddr.sin_addr.s_addr = inet_addr("175.24.78.187");

  //1.创建套接字
   int serverId = socket(PF_INET,SOCK_STREAM,0);
   if(serverId<0)
   {
    perror("serverFd\n");
    return  -1;
   }
   printf("socket ok\n");
  //2.绑定地址信息
  int ret = bind(serverId,(struct sockaddr *)&myaddr,sizeof(myaddr));
  if(ret<0)
  {
    perror("bind\n");
    close(serverId);
    return -1;
  }
   printf("bind ok\n");

   //设置连接密码
  char buffer[BUFFER_SIZE];    
  bzero(buffer, BUFFER_SIZE);
  char password[FILE_NAME_MAX_SIZE+1];    
  bzero(password, FILE_NAME_MAX_SIZE+1); 
  printf("请输入本次的连接密码:\t");    
  scanf("%s", password);

  //3.创建一个监听队列
    if(listen(serverId,10)<0)
    {
      perror("listen\n");
      close(serverId);
      return -1;
    }
    printf("listening....\n");
  //4.接受链接请求
  int conId=accept(serverId,NULL,NULL);
  
  //接收客户端输入的连接码
  //bzero(buffer, BUFFER_SIZE);
  ret = recv(conId,buffer,sizeof(buffer),0);  //接收密码
  if(ret<0)
  {
      perror("recv\n");
      close(serverId);
      close(conId);
      return -1;
  }
  // 然后从buffer(缓冲区)拷贝到cpassword中        
  char cpassword[FILE_NAME_MAX_SIZE+1];        
  bzero(cpassword, FILE_NAME_MAX_SIZE+1);         //将filename清零
  strncpy(cpassword, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buffer));
  printf("%s\n", cpassword);

  //比较
  char *p=password;
  char *cp=cpassword;
  if(strcmp(p,cp)!=0)
  {
       conId=-1;
       printf("连接码错误");
  }
  if(conId<0)
  {
    perror("accept\n");
    close(serverId);
    return -1;
  }

  printf("accept ok\n");

  
  
  /*
  //5.收发消息
  */
  chat(conId,serverId,ret);

  
  //6.收发文件
  //sendfile(conId,serverId,ret);
  
          // 关闭与客户端的连接        
    // close(new_server_socket_fd);
  
  //只发送文件
  //sendfile(conId);

  //只接收文件
  //recvfile(conId,serverId,ret);
  
  //关闭套接字
  close(serverId);
  close(conId);
  return 0;
}

void sendfile(int conId)      //发送文件
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
  if(send(conId, buf, BUFFER_SIZE, 0) < 0)    
  {        
      perror("Send File Name Failed:");        
      exit(1);    
  }
  printf("文件名发送完成，准备发文件\n");
  printf("睡眠10秒\n");
  sleep(10);

  bzero(buf, BUFFER_SIZE);
  // 打开文件并读取文件数据        
  FILE *fp = fopen(file_name, "r");        
  if(NULL == fp)        
  {            
      printf("File:%s Not Found\n", file_name);        
  }        
  else        
  {            
      printf("开始传输");
      bzero(buf, BUFFER_SIZE);      //将buf清零      
      int length = 0;            
      // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止            
      while((length = fread(buf, sizeof(char), BUFFER_SIZE, fp)) > 0)            
      {              
          printf("进入循环");
          if(send(conId, buf, length, 0) < 0)                
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

void recvfile(int conId,int serverId,int ret) //接收文件
{
  char buffer[BUFFER_SIZE];
  memset(buffer,0,BUFFER_SIZE);  //将buf清零
  ret = recv(conId,buffer,sizeof(buffer),0);  //接收文件名
  if(ret<0)
  {
      perror("recv\n");
      close(serverId);
      close(conId);
      return -1;
  }
  // 然后从buffer(缓冲区)拷贝到file_name中        
  char file_name[FILE_NAME_MAX_SIZE+1];        
  bzero(file_name, FILE_NAME_MAX_SIZE+1);         //将filename清零
  strncpy(file_name, buffer, strlen(buffer)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buffer));
  printf("文件名为：");
  printf("%s\n", file_name);

  // 打开文件，准备写入  
  printf("打开文件");  
    FILE *fp = fopen(file_name, "w"); 
    printf("成功打开文件");   
    if(NULL == fp)    
    {        
        printf("File:\t%s Can Not Open To Write\n", file_name);        
        exit(1);    
    }     
    else
    {
        // 从服务器接收数据到buffer中    
        // 每接收一段数据，便将其写入文件中，循环直到文件接收完并写完为止  
        printf("开始写入文件");  
        bzero(buffer, BUFFER_SIZE);    
        int length = 0;    
        while((length = recv(conId, buffer, BUFFER_SIZE, 0)) > 0)    
        {      
            printf("进入循环");  
            if(fwrite(buffer, sizeof(char), length, fp) < length)        
            {            
                printf("File:\t%s Write Failed\n", file_name);            
                break;        
            }        
            bzero(buffer, BUFFER_SIZE);    
        }        
        printf("Receive File:\t%s save Successful!\n", file_name);
    }    
    close(fp); //关闭文件
}


void sendrecvfile(int conId,int serverId,int ret)
{
  while(1)
  {
    char buf[BUFFER_SIZE];
    memset(buf,0,BUFFER_SIZE);  //将buf清零
    ret = recv(conId,buf,sizeof(buf),0);
    if(ret<0)
    {
        perror("recv\n");
        close(serverId);
        close(conId);
        return -1;
    }
    // 然后从buffer(缓冲区)拷贝到file_name中        
    char file_name[FILE_NAME_MAX_SIZE+1];        
    bzero(file_name, FILE_NAME_MAX_SIZE+1);         //将filename清零
    strncpy(file_name, buf, strlen(buf)>FILE_NAME_MAX_SIZE?FILE_NAME_MAX_SIZE:strlen(buf));        
    printf("%s\n", file_name);         

    // 打开文件并读取文件数据        
    FILE *fp = fopen(file_name, "r");        
    if(NULL == fp)        
    {            
        printf("File:%s Not Found\n", file_name);        
    }        
    else        
    {            
        bzero(buf, BUFFER_SIZE);      //将buf清零      
        int length = 0;            
        // 每读取一段数据，便将其发送给客户端，循环直到文件读完为止            
        while((length = fread(buf, sizeof(char), BUFFER_SIZE, fp)) > 0)            
        {              
            if(send(conId, buf, length, 0) < 0)                
            {                   
                printf("Send File:%s Failed./n", file_name);                    
                break;                
            }                
            bzero(buf, BUFFER_SIZE);     //将buf清零      
        }             
        // 关闭文件            
        fclose(fp);            
        printf("File:%s Transfer Successful!\n", file_name);        
    }
  }
}

void chat(int conId,int serverId,int ret)
{
  while(1)
  {
      char buf[BUFFER_SIZE];
      memset(buf,0,BUFFER_SIZE);
      printf("please input message (to xldclient)\n");
       gets(buf);
       if(strcmp(buf,"quit")==0)
       {
         break;
       }

      //记录服务器发给客户端的信息
      char *from1="server";
      char *from2="client";
      ret = send(conId,buf,sizeof(buf),0);
      if(ret <0)
      {
          perror("send\n");
          close(serverId);
          close(conId);
          return -1; 
      }

      char *message=buf;
      if(strcmp(message,"send")==0)
      {
        
        //bzero(buf, BUFFER_SIZE);
        sendfile(conId);    //conId

      }

      recard(from1,message);
      bzero(buf, BUFFER_SIZE);
      ret = recv(conId,buf,sizeof(buf),0);
      if(ret<0)
      {
          perror("recv\n");
          close(serverId);
          close(conId);
          return -1; 
      }
      if(strcmp(buf,"send")==0)
      {
        recvfile(conId,serverId,ret);
      }
      printf("from xldclient:%s\n",buf);
      recard(from2,message);  //记录客户端发过来的信息
  }
}

void recard(char *from,char *message) //记录聊天信息
{  
    time_t t;
    struct tm * lt;
    FILE* fp=NULL;
    time (&t);//获取Unix时间戳。
    lt = localtime (&t);//转为时间结构。
    //printf ( "%d/%d/%d %d:%d:%d\n",lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec);//输出结果

    fp=fopen("./time.txt","a");//打开（创建）time.txt文件
    if(fp==NULL)//判断是否打开成功
    {
            perror("fopen");
            return -1;
    }
    else
    {
        fprintf(fp,"%s: %d/%d/%d %d:%d:%d - %s\n",from,lt->tm_year+1900, lt->tm_mon, lt->tm_mday, lt->tm_hour, lt->tm_min, lt->tm_sec,message);//在打开的time.txt文件中写入日期和时间
    }
    fclose(fp);
}