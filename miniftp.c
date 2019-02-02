////////////////////////////////////
// Mini Ftp client
// Done by Benjamin Ang
// A0156065E
// CS3103 Assignment Part C 
////////////////////////////////////


#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <sys/ioctl.h>

struct hostent *host;

void upload(int);
void download_file(int);
void quit(int);
void list_dir(int);
void change_dir(int);
void print_dir(int);
int mk_dataconnetion(int);
void get_reply(int);
int connection();
int menu();
int main(int argc, char *argv[])  
{        
	int option,sock;

	do{
		option = menu();
		switch(option)
		{
			case 1 :
				sock = connection();
				break;

			case 2 :
				print_dir(sock);
				break;

			case 3:
				change_dir(sock);
				break;

			case 4: 
				list_dir(sock);
				break;

			case  5:
				upload(sock);
				break;

			case  6:
				download_file(sock);
				break;

			case 7:
				quit(sock);
				break;	       

		}

	}while(option!=7);
	return 0;
}
int menu()
{
	int i,option;
	for(i=0;i<42;i++){
		printf("*");
	}
	printf("\n");
	for(i=0;i<14;i++){
		printf("*");
	}
	printf("Mini-FTP client");
	for(i=0;i<13;i++){
		printf("*");
	}
	printf("\n");
	for(i=0;i<42;i++){
		printf("*");
	}
	printf("\n");
	printf("1. Connect to FTP server\n");
	printf("2. Print working directory\n");
	printf("3. Change working directory\n");
	printf("4. List all files\n");
	printf("5. Upload file\n");
	printf("6. Download file\n");
	printf("7. Quit\n");
	printf("Enter option (1-7): ");
	scanf("%d",&option);
	return option;   
}
void quit(int sock){

	char Exitcmd[50] = "QUIT\n";
	if(send(sock,Exitcmd,strlen(Exitcmd),0)<0){
		printf("Exit cmd failed");
		exit(1);
	}
	get_reply(sock);
	shutdown(sock, SHUT_RDWR);
	close(sock); 

}
int connection()
{	
	int sock;
	struct sockaddr_in addr,client;
	in_port_t port;     
	int in_port;
	char hostname[30];
	char user[100];
	char password[100];
	char src1[50];
	char src2[50];
	strcpy(src1, "User ");
	strcpy(src2, "Pass ");
	printf("Specify hostname: ");
	scanf("%s",hostname);
	if((host = gethostbyname(hostname)) == NULL ) {
		herror("gethosterror");
		exit(1);
	}
	printf("Enter port: ");
	scanf("%d",&in_port);
	port = in_port;

	//copy over info to socket struct
	bcopy(host->h_addr, &addr.sin_addr, host->h_length);
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;

	//create a socket
	sock = socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&client,0,sizeof(struct sockaddr_in));
	client.sin_family = AF_INET;
	client.sin_addr.s_addr = htonl(INADDR_ANY);
	client.sin_port = 0;
	if(bind(sock,(struct sockaddr *)&client,sizeof(struct sockaddr_in))<0){

		perror("cannot bind");
		close(sock);
		exit(1);
	}

	if(connect(sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){

		perror("connect");
		exit(1);
	}else{

		printf("Connection success\n");
	}

	get_reply(sock);

	printf("USER: ");
	scanf("%s",user);
	strcat(user, "\n");
	strcat(src1,user);
	printf("\n");

	if(send(sock,src1,strlen(src1),0)<0){
		printf("send user failed");
		exit(1);
	}
	get_reply(sock);

	printf("PASS: ");
	scanf("%s",password);
	strcat(password, "\n");
	strcat(src2,password);
	printf("\n");

	if(send(sock,src2,strlen(src2),0)<0){
		printf("send password failed");
		exit(1);
	}
	get_reply(sock);
	connection_flag = 1;
	return sock;
}
int mk_dataconnection(int sock){
	char Pasvcmd[50]="EPSV\n";
	int data_port,data;
	in_port_t dataport;
	struct sockaddr_in data_addr;
	struct sockaddr_in client;

	if(send(sock,Pasvcmd,strlen(Pasvcmd),0)<0){
		printf("send passive cmd failed");
		exit(1);
	}
	get_reply(sock);

	printf("Enter port given by server: ");
	scanf("%d",&data_port);
	printf("\n");


	dataport= data_port;
	bcopy(host->h_addr, &data_addr.sin_addr, host->h_length);
	data_addr.sin_port = htons(dataport);
	data_addr.sin_family = AF_INET;


	data = socket(AF_INET, SOCK_STREAM, 0);
	memset((char *)&client,0,sizeof(struct sockaddr_in));
	client.sin_family = AF_INET;
	client.sin_addr.s_addr = htonl(INADDR_ANY);
	client.sin_port = 0;
	if(bind(data,(struct sockaddr *)&client,sizeof(struct sockaddr_in))<0){

		perror("cannot bind");
		close(data);
		exit(1);
	} 
	if(connect(data, (struct sockaddr *)&data_addr, sizeof(struct sockaddr_in)) == -1){

		perror("connect");
		exit(1);
	}else{

		//	printf("Data Connection success\n");
	}

	return data;
}
void list_dir(int sock){
	char Listcmd[50] = "LIST\n";
	int data;

	data = mk_dataconnection(sock);

	if(send(sock,Listcmd,strlen(Listcmd),0)<0){
		printf("send list cmd failed");
		exit(1);
	}
	get_reply(sock);
	get_reply(data);
	shutdown(data, SHUT_RDWR);
	close(data); 
	get_reply(sock);	
}
void print_dir(int sock){
	char Printcmd[50] = "PWD\n";

	if(send(sock,Printcmd,strlen(Printcmd),0)<0){
		printf("print working directory failed");
		exit(1);
	}
	get_reply(sock);
}
void change_dir(int sock){
	char Changecmd[50] = "CWD ";
	char dir[50];

	printf("\n");
	printf("Which directory? ");
	scanf("%s",dir);
	strcat(Changecmd,dir);
	strcat(Changecmd,"\n");	
	if(send(sock,Changecmd,strlen(Changecmd),0)<0){
		printf("change working directory failed");
		exit(1);
	}
	get_reply(sock);
}
void get_reply(int sock){
	char buffer[1000000];
	int bytesRead;
	do{
		memset(buffer,0,sizeof(buffer));
		bytesRead = recv(sock,buffer,1000000,0);
		buffer[bytesRead] = '\0';
		printf("%s",buffer);
	}while(buffer[bytesRead-1]!='\n');
	printf("\n");

}	
void download_file(int sock){
	char Retrcmd[50] = "RETR ";
	char file[50];
	int data;
	FILE *fp;
	printf("File to retrieve ? " );
	scanf("%s",file);
	strcat(Retrcmd,file);
	strcat(Retrcmd,"\n");
	data = mk_dataconnection(sock);
	if(send(sock,Retrcmd,strlen(Retrcmd),0)<0){
		printf("Retr cmd failed");
		exit(1);
	}

	get_reply(sock);

	char buffer[255];
	int ch = 0;
	fp = fopen(file,"a");
	long words;

	fp = fopen(file,"wt");
	sleep(5);	
	ioctl(data,FIONREAD,&words);
	do{
		memset(buffer,0,sizeof(buffer));
		printf("Bytes received: %ld\n",words);
		while(ch!=words){

			read(data,buffer,1);
			fprintf(fp, "%s", buffer);
			memset(buffer,0,sizeof(buffer));
			ch++;


		}
		ch = 0;
		ioctl(data,FIONREAD,&words);
	}while(words!=0);

	fclose(fp);
	shutdown(data, SHUT_RDWR);
	close(data);

	get_reply(sock);

}
void upload(int sock){
	char Strcmd[50] = "STOR "; 
	char file[50];
	char buffer[255];
	int data,lsize;
	int ch = 0;
	int pos = 0;
	FILE *fp;
	printf("File to send ? " );
	scanf("%s",file);
	strcat(Strcmd,file);
	strcat(Strcmd,"\n");
	data = mk_dataconnection(sock);
	if(send(sock,Strcmd,strlen(Strcmd),0)<0){
		printf("Str cmd failed");
		exit(1);
	}
	get_reply(sock);
	fp = fopen(file,"rt");
	fseek(fp,0,SEEK_END);
	lsize = ftell(fp);
	rewind(fp);
	memset(buffer,0,sizeof(buffer));
	while(ch!=lsize){
		fseek(fp,pos,SEEK_SET);
		fread(buffer,1,1,fp);
		if(send(data,buffer,strlen(buffer),0)<0){
			printf("Send byte failed");
			exit(1);
		}
		ch++;
		pos++; 
		memset(buffer,0,sizeof(buffer));
	}
	printf("File has been sent\n");
	fclose(fp);
	shutdown(data, SHUT_RDWR);
	close(data);

	get_reply(sock);       	

}
