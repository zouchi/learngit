#include<iostream>
#include<sstream>
#include<string>
#include<conio.h>
//#include<cstring>
using namespace std;

#define N 42

typedef struct//i结点
{
	char name[8];
	char owner[8];
	int isIndex;
	int size;
	char right[8];
    char * addr[5];
	char * last_index;
}iNode;

typedef struct link_i
{
	iNode inode;
	link_i *next;
}link_iNode;

typedef struct //文件目录项
{
	char name[8];
	int ID;//i结点所在物理块号
}INDEX;

typedef struct 
{
	string name;
	/*string code;*/
	char code[8];
}User;

User user[2];
char bitmap[400],i_map[111];
INDEX index[N];//目录表数组
char *p,*i_area,*block;//分别指向i结点区首地址和物理存储区首地址
iNode inode;
char *present_index;//当前目录表地址
char *temp;//临时目录

char list[10][10],full_index[10][10],temp_index[10][10];
int len,f_index_len,state=0;

link_iNode *active[2];//分别表示user1,user2的活动i结点链表的头指针
link_iNode *sys;//系统打开文件表头指针

void init(char *p)
{
	for(int i=0;i<400;i++) bitmap[i]='0';
	bitmap[0]='1';bitmap[1]='1';
	char *s=bitmap;
	for(i=0;i<400;i++) *(p+i)=*(s+i);

	for(i=0;i<112;i++) i_map[i]='0';
	i_map[0]='1';
	s=i_map;int j;
	for(i=400,j=0;i<512;i++,j++) *(p+i)=*(s+j);

	i_area=p+512;//i节点区首地址
	block=p+512*112;//物理块首地址
	present_index=block;//当前目录表地址
	temp=present_index;
	strcpy(full_index[0],"root"); 
	f_index_len=1;               //初始化当前路径
	for(i=1;i<10;i++) strcpy(full_index[i],"");
	

//---------给根目录的i结点赋值-------------------
	strcpy(inode.name,"root");
	strcpy(inode.owner,"admin");
	inode.size=0;
	strcpy(inode.right,"public");
	inode.addr[0]=512*112+p;
	inode.addr[1]=512*113+p;inode.addr[2]=NULL;inode.addr[3]=NULL;inode.addr[4]=NULL;
	inode.isIndex=1;
	inode.last_index=NULL;
	s=(char *)&inode;
	for(i=0;i<sizeof(iNode);i++) *(i_area+i)=*(s+i);

//----------给根目录赋值--------------------------
	index[0].ID=1;
	strcpy(index[0].name,"root");
	for(i=1;i<N;i++) {index[i].ID=0;strcpy(index[i].name,"");}
	s=(char *)&index[0];
	for(i=0;i<sizeof(INDEX)*N;i++) *(block+i)=*(s+i);
}

void f_init(char *p)
{
	char *s=bitmap;
	for(int i=0;i<sizeof(bitmap);i++) *(s+i)=*(p+i);
	s=i_map;
	for(int j=0;j<sizeof(i_map);j++) *(s+j)=*(p+i+j);

	i_area=p+512;//i节点区首地址
	block=p+512*112;//物理块首地址
	present_index=block;//当前目录表地址
	temp=present_index;
	strcpy(full_index[0],"root"); 
	f_index_len=1;               //初始化当前路径
	for(i=1;i<N;i++) strcpy(full_index[i],"");
}


int logon()
{
	while(1)
	{
		string name;
		char code[8],ch;
		cout<<"logon:";
		cin>>name;
		for(int i=0;i<2;i++){if(user[i].name==name) break;}
		if(i==2) {cout<<"没有该用户!\n";continue;}
		cout<<"password:";
		int j=0;ch=getch();
		while(ch!='\r') 
		{
			code[j++]=ch;
			putchar('*');
			ch=getch();
		}
		code[j]='\0';
		cout<<endl;
        if(strcmp(user[i].code,code)==0) return i+1;
		else {cout<<"密码不正确!\n";return 0;}
	}
}

void depart(char * e,char list[][10],int *len)
{
	int j=-1,k=0;
	for(int i=0;e[i]!='\0';i++)
	{
		if(e[i]=='/')
		{
			if(j>=0) list[j][k]='\0';
			j++;
			k=0;
			continue;
		}
		if(j<0) j=0;
		list[j][k++]=e[i];
	}
	list[j][k]='\0';
	*len=j+1;	
	if(e[0]!='/'&&e[0]!='.')
	{
		for(i=0;i<f_index_len;i++) strcpy(temp_index[i],full_index[i]);
		for(i=0;i<=j;i++) strcpy(temp_index[i+f_index_len],list[i]);
		*len=f_index_len+j+1;
		for(i=0;i<10;i++) strcpy(list[i],"");
		for(i=0;i<*len;i++) strcpy(list[i],temp_index[i]); 
	}
	if(e[0]=='.'&&e[1]=='.')
	{
		char *s=(char *)&index;
		for(int i=0;i<sizeof(INDEX)*N;i++) *(s+i)=*(present_index+i);
		s=(char *)&inode;
		for(i=0;i<sizeof(iNode);i++) *(s+i)=*(index[0].ID*512+i+p);
		s=(char *)&index;          //cout<<"inode.last_index-block/512="<<(inode.last_index-block)/512<<endl;
		for(i=0;i<sizeof(INDEX)*N;i++) *(s+i)=*(inode.last_index+i);

		if(strcmp(index[0].name,"root")!=0) 
		{
			strcpy(list[0],index[0].name);
	    	for(i=0;i<f_index_len-1;i++) strcpy(temp_index[i],full_index[i]);
	    	for(i=0;i<=j;i++) strcpy(temp_index[i+f_index_len-1],list[i]);
	    	*len=f_index_len+j;
	    	for(i=0;i<*len;i++) strcpy(list[i],temp_index[i]);
		}
		else 
		{
			for(i=0;i<f_index_len-1;i++) strcpy(list[i],full_index[i]);
			*len=f_index_len-1;
		}
	}          //for(i=0;i<*len;i++) cout<<"list[i]   "<<list[i]<<endl;
}

int find(char list[][10],int *len)
{
	int j=0,flag=0;
	temp=block;
	char *s=(char *)&index,*t;   //for(int i=0;i<sizeof(INDEX)*10;i++) *(s+i)=*(block+i);
	                             //for(i=0;i<10;i++)cout<<index[i].name<<"*********\n";
	                             //cout<<"find    temp-block/512="<<(temp-block)/512<<endl; 
//--------------------------------------------------------
while(j<*len-1)
{                
	for(int i=0;i<sizeof(INDEX)*N;i++) *(s+i)=*(temp+i);
	
	for(i=0;i<N;i++)
	{                           
		if(strcmp(index[i].name,list[j])==0)
		{
			t=(char *)&inode;
			for(int k=0;k<sizeof(iNode);k++) *(t+k)=*(index[i].ID*512+k+p);
			if(inode.isIndex==1)
			{
				temp=inode.addr[0];
		     	j++;
				flag=1;
				break;
			}
		}
	}
	if(flag==0) return 0;
}
//---------------------------------------------------------
    return 1;
}

void create()
{
	string extra;
	cin>>extra;
	char e[30];int flag=0;
	strcpy(e,extra.c_str()); 
	depart(e,list,&len);             
	flag=find(list,&len);
	if(flag==0) cout<<"目录错误!\n";
	else
	{
		char *s=(char *)&index;
		for(int i=0;i<sizeof(INDEX)*N;i++) *(s+i)=*(temp+i);   
		for(i=0;i<N;i++) 
		{
			if(index[i].ID==0)
			{
				strcpy(index[i].name,list[len-1]);
				for(int j=0;j<sizeof(i_map);j++) //分配i结点
				{
					if(i_map[j]=='0') 
					{
						index[i].ID=j+1;
						i_map[j]='1';
						*(p+400+j)='1';
						break;
					}
				}   
				int g=0;       
				for(int k=0;k<sizeof(bitmap);k++)//给对应i结点赋值,并分配物理块
				{
					if(bitmap[k]=='0'&&g==0)
					{
				    	strcpy(inode.name,list[len-1]);              //cout<<"list[len-1]"<<list[len-1]<<endl;
						char n[10];
						strcpy(n,user[state-1].name.c_str());
						strcpy(inode.owner,n);
						inode.isIndex=0;
						inode.size=0;
						inode.addr[0]=(112+k)*512+p;
						for(int m=1;m<5;m++) inode.addr[m]=NULL;
						inode.last_index=temp;
						bitmap[k]='1';
						*(p+k)='1';
						g=1;continue;
					}
					if(bitmap[k]=='0'&&g==1) 
					{
						inode.addr[1]=(112+k)*512+p;
						bitmap[k]='1';
						*(p+k)='1';
						break;
					}
				}
				s=(char *)&inode;            
				for(k=0;k<sizeof(inode);k++) *(512*(j+1)+k+p)=*(s+k);//写回512字节的内存中

				for(k=i+1;k<N;k++) index[k].ID=0;
				s=(char *)&index;
				for(k=0;k<sizeof(INDEX)*N;k++) *(temp+k)=*(s+k);//将目录写回512字节的内存中
				
				/*s=(char *)&index[0];
				for(k=0;k<sizeof(INDEX)*10;k++) *(s+k)=*(block+k);
				for(k=0;k<10;k++) cout<<index[k].ID<<"    "<<index[k].name<<"**********\n";*/
				break;
			}
		}
	}
}

void link_del(link_iNode *m,char* name)
{
	link_iNode *pre=m,*s=pre->next;
	while(s!=NULL)
	{
		if(strcpy(s->inode.name,name)==0)
		{
			pre->next=s->next;free(s);break;//
		}
		pre=s;s=s->next;
	}
}

void link_add(link_iNode * m,iNode *inode)
{
	link_iNode * t=(link_iNode *)malloc(sizeof(link_iNode));
	strcpy(t->inode.name,inode->name);
	strcpy(t->inode.owner,inode->owner);
	t->inode.isIndex=inode->isIndex;
	t->inode.size=inode->size;
	for(int k=0;k<5;k++) t->inode.addr[k]=inode->addr[k];
	t->next=m->next;
	m->next=t;//
}

link_iNode* link_find(link_iNode * m,iNode * inode,char * name)
{
	link_iNode *s=m->next;//cout<<"link_find="<<name<<"*********\n";
	while(s!=NULL)
	{                       //cout<<"s->inode.name="<<s->inode.name<<"*********\n";
		if(strcmp(s->inode.name,name)==0)
		{
			strcpy(inode->name,s->inode.name);
			strcpy(inode->owner,s->inode.name);
			inode->size=s->inode.size;
			inode->isIndex=s->inode.isIndex;
			for(int i=0;i<5;i++) inode->addr[i]=s->inode.addr[i];
			return s;
		}
		s=s->next;
	}
	return NULL;
}


void del()
{
	string extra;
	cin>>extra;
	char e[30];
	strcpy(e,extra.c_str());
	depart(e,list,&len);
	int flag=0;
	flag=find(list,&len);
	if(flag==0) cout<<"文件不存在!\n";
	else
	{
		char *s=(char *)&index;
		for(int i=0;i<sizeof(index);i++) *(s+i)=*(temp+i);
		for(i=0;i<N;i++)
		{
			if(strcmp(index[i].name,list[len-1])==0)
			{
				s=(char *)&inode;
				for(int j=0;j<sizeof(iNode);j++) *(s+j)=*(index[i].ID*512+j+p);
				//cout<<(inode.addr[0]-p)/512<<"_________________index[i].ID="<<index[i].ID<<endl;
				bitmap[(inode.addr[0]-p)/512-112]='0';//释放文件占用的物理块
				bitmap[(inode.addr[1]-p)/512-112]='0';
				*(p+(inode.addr[0]-p)/512-112)='0';
				*(p+(inode.addr[1]-p)/512-112)='0';

				link_del(active[state-1],index[i].name);//若文件已经打开,在活动i结点中把该文件删除
				link_del(sys,index[i].name);//在系统打开文件表中把该文件删除
             
				i_map[index[i].ID-1]='0';
				*(char *)(index[i].ID-1+p+400)='0';//释放i结点
				strcpy(index[i].name,"");
				index[i].ID=0;
				s=(char *)&index[0];
				for(j=0;j<sizeof(index);j++) *(temp+j)=*(s+j);
				break;
			}
		}
	}
}

void open()
{
	string extra;
	char e[30];
	int flag=0;
	cin>>extra;
	strcpy(e,extra.c_str());
	depart(e,list,&len);
	flag=find(list,&len);           //cout<<flag<<"flag**********\n";cin>>extra;
	if(flag==0) cout<<"未找到该文件!\n";
	else
	{
		char *s=(char *)&index;
		for(int i=0;i<sizeof(index);i++) *(s+i)=*(temp+i);//cout<<temp<<"   temp\n";
		for(i=0;i<N;i++)
		{
			if(strcmp(index[i].name,list[len-1])==0)
			{
				s=(char *)&inode;   // cout<<"index["<<i<<"].ID="<<index[i].ID<<"*********\n";
				for(int j=0;j<sizeof(iNode);j++) *(s+j)=*(index[i].ID*512+j+p);   //cout<<inode.name<<"  "<<inode.size<<endl;
				link_add(active[state-1],&inode);
				link_add(sys,&inode);
				cout<<"成功打开!\n";
				break;
			}
		}
	}
}

void close()
{
	string extra;
	char list[10][10],e[30];
	int flag=0;
	strcpy(e,extra.c_str());
	depart(e,list,&len);
	flag=find(list,&len);
	if(flag==0) cout<<"未找到该文件!\n";
	else
	{
    	link_del(active[state-1],list[len-1]);
		link_del(sys,list[len-1]);
		cout<<"关闭完成!\n";
	}
}

void read()
{
	string extra;
	cin>>extra;
	char list[10][10],e[30];
	link_iNode * flag=NULL;
	strcpy(e,extra.c_str());
	depart(e,list,&len);    //cout<<"list[len-1]="<<list[len-1]<<endl;
	flag=link_find(active[state-1],&inode,list[len-1]);    
	if(flag==NULL) cout<<"未在用户打开文件表中找到该文件!\n";
	else
	{
		if(inode.size==0) cout<<"the file is empty!\n";
		else 
		{
			for(int k=0;k<inode.size;k++)
			{
    			if(k%60==0) cout<<'\n';
    			cout<<(char)(*(inode.addr[0]+k));
			}
    		cout<<"\n\n";
		}
	}
}

void write()
{
	string extra;
	cin>>extra;
	char list[10][10],e[512],content[512],ch;
	link_iNode *flag=NULL;
	strcpy(e,extra.c_str());
	depart(e,list,&len);
	flag=link_find(active[state-1],&inode,list[len-1]);
	if(flag==NULL) cout<<"未在用户打开文件表中找到该文件!\n";
	else
	{
		cout<<"输入文件内容:\n";
		ch=getchar();ch=getchar();                  //strcpy(e,extra.c_str());
		int i=0;
		while(ch!=4) {content[i++]=ch;ch=getchar();}
		content[i]='\0';
		for(int j=0;j<=i;j++) (char)(*(flag->inode.addr[0]+j))=content[j];
//cout<<flag->inode.name<<"     write file name";
		flag->inode.size=i;
		inode.size=i;

		find(list,&len);//cout<<(temp-block)/512<<"   (temp-block)/512  OOOOOOOO\n";
		char *s=(char *)&index;
		for(j=0;j<sizeof(index);j++) *(s+j)=*(temp+j);
		/*int l=(len-2>=0?len-2:0);
		for(j=1;j<10;j++) 
		{
			if(strcmp(index[j].name,list[l])) break;
		}
		s=(char *)&inode;
		for(i=0;i<sizeof(iNode);i++) *(s+i)=*(index[j].ID*512+p+i);// atention   jjjjjjjjjjjjj
		s=(char *)&index[0];
		for(i=0;i<sizeof(INDEX)*10;i++) *(s+i)=*(inode.addr[0]+i);*/
		for(j=0;j<N;j++)
		{           
			if(strcmp(index[j].name,list[len-1])==0) 
			{
				*(int *)(index[j].ID*512+20+p)=i;
				break;
			}
		}
	}
}

void mkdir()
{
	string extra;
	cin>>extra;
	char list[10][10];
	char e[30];
	int flag=0;
	strcpy(e,extra.c_str());
	depart(e,list,&len);    for(int m=0;m<len;m++) cout<<list[m]<<"********\n";
	flag=find(list,&len);   //cout<<"after find\n";
	if(!flag) cout<<"不存在此目录!\n";
	else
	{
		char *s=(char *)&index;   //cout<<"mkdir    temp-block/512="<<(temp-block)/512<<endl;
		for(int i=0;i<sizeof(index);i++) *(s+i)=*(temp+i);  //for(i=0;i<10;i++)cout<<index[i].name<<"******\n";
		for(i=1;i<N;i++)
		{
			if(index[i].ID==0) 
			{                           //cout<<"list[len-1]"<<list[len-1]<<endl;
				strcpy(index[i].name,list[len-1]);
				for(int j=0;j<sizeof(i_map);j++) //分配i结点
				{
					if(i_map[j]=='0') 
					{
						index[i].ID=j+1;
						i_map[j]='1';
						*(p+400+j)='1';
						break;
					}
				}
				int g=0;
				for(int k=0;k<sizeof(bitmap);k++)//给对应i结点赋值,并分配物理块
				{
					if(bitmap[k]=='0'&&g==0)
					{
				    	strcpy(inode.name,list[len-1]);
						if(state==1) strcpy(inode.owner,"user1");
						else strcpy(inode.owner,"user1");
						inode.isIndex=1;
						inode.size=0;
						inode.addr[0]=(112+k)*512+p;
						for(int m=1;m<5;m++) inode.addr[m]=NULL;
						inode.last_index=temp;
						g=1;
						bitmap[k]='1';
						*(p+k)='1';
						continue;
					}
					if(bitmap[k]=='0'&&g==1)
					{
						inode.addr[1]=(112+k)*512+p;
						bitmap[k]='1';
						*(p+k)='1';
						break;
					}
				}
				s=(char *)&inode;
				for(k=0;k<sizeof(inode);k++) *(512*(j+1)+k+p)=*(s+k);//将inode写回512字节的内存中
				s=(char *)&index[0];
				for(k=0;k<sizeof(index);k++) *(temp+k)=*(s+k);//将目录写回512字节的内存中
				
				/*cout<<"mkdir    temp-block/512="<<(temp-block)/512<<endl;
				for(k=0;k<10;k++) cout<<index[k].name<<"   ***************\n";*/

				for(k=0;k<N;k++) {strcpy(index[k].name,"");index[k].ID=0;}
				strcpy(index[0].name,list[len-1]);index[0].ID=j+1;
				s=(char *)&index[0];
				for(k=0;k<sizeof(index);k++) *(inode.addr[0]+k)=*(s+k);

                /*cout<<"inode.addr[0]-block/512="<<(inode.addr[0]-block)/512<<endl;
				for(k=0;k<10;k++) cout<<index[k].name<<"   &&&&&&&&&&&&&&&\n";*/
				break;
			}
		}
	}
}

void dir()
{
	char *s=(char *)&index;                              
	for(int i=0;i<sizeof(index);i++) *(s+i)=*(present_index+i);
	//cout<<present_index<<"   (present_index)\n";
	for(i=1;i<N;i++)//改称i从1开始,第零个是目录名
	{
		if(index[i].ID!=0) cout<<index[i].name<<endl;    //cout<<index[i].name<<"  dir dir\n";
	}
}

void cd()
{
	string extra;
	cin>>extra;
	char list[10][10],e[30];
	char *s;
	int flag=0;
	strcpy(e,extra.c_str());
	depart(e,list,&len);  //for(int l=0;l<5;l++) cout<<list[l]<<"*************\n";

	flag=find(list,&len);

	if(!flag) cout<<"不存在此目录!\n";
	else
	{
		s=(char *)&index;
		for(int i=0;i<sizeof(index);i++) *(s+i)=*(temp+i); 
		//for(i=0;i<5;i++) cout<<"temp   index[i]"<<index[i].name<<"&&&&&&&&&&&\n";
		if(strcmp(list[len-1],"root")==0)
		{
			present_index=p+512*112;
			for(i=1;i<N;i++) strcpy(full_index[i],"");
			strcpy(full_index[0],"root");
			f_index_len=1;//
		}
		else
	{//-----------------------------------------------------
    	for(i=1;i<N;i++)
		{     
			if(strcmp(index[i].name,list[len-1])==0)
			{       //cout<<"**************";
				s=(char *)&inode;
				for(int j=0;j<sizeof(iNode);j++) *(s+j)=*(index[i].ID*512+j+p);
				if(inode.isIndex==1) 
				{             //cout<<"inode.name="<<inode.name<<"*************\n";
					present_index=inode.addr[0];
					for(j=0;j<len;j++) strcpy(full_index[j],list[j]);
					f_index_len=len;
					break;
				}
				else cout<<"error!!!!!\n";
			}
		}
		if(i==10) cout<<"目录错误!\n";
	}//------------------------------------------------------------------
	}
}

void logout()
{
	present_index=p+512*112;
	for(int i=1;i<N;i++) strcpy(full_index[i],"");
	strcpy(full_index[0],"root");
	f_index_len=1;//
	state=0;
	while(!state) state=logon();
}

int execute()
{
	int i=0;
	for(i=0;i<f_index_len;i++) cout<<"/"<<full_index[i];cout<<">";
	string opera;char n[10];
	cin>>opera;
	strcpy(n,opera.c_str());
	if(strcmp(n,"create")==0) create();
	else if(strcmp(n,"open")==0) open();
	else if(strcmp(n,"read")==0) read();
	else if(strcmp(n,"write")==0) write();
	else if(strcmp(n,"close")==0) close();
	else if(strcmp(n,"delete")==0) del();
	else if(strcmp(n,"mkdir")==0) mkdir();
	else if(strcmp(n,"cd")==0) cd();
	else if(strcmp(n,"dir")==0) dir();
	else if(strcmp(n,"logout")==0) logout();
	else if(strcmp(n,"quit")==0) return 0;
	else cout<<"there is an error!!!\n";

	return 1;
}

void main()
{
	FILE *fp;
	p=(char *)malloc(512*512);
//	fp=fopen("os.txt","r+");
	if((fp=fopen("os.txt","r+"))==NULL) 
	{
		cout<<"can't open file!";
		init(p);
	}
	else 
	{
		fread(p,1,512*512,fp);
    	f_init(p);  
	}  //init(p);                    //if(present==block) cout<<"================\n";getchar();
	user[0].name="user1";strcpy(user[0].code,"123");
	user[1].name="user2";strcpy(user[1].code,"123");

	active[0]=(link_iNode *)malloc(sizeof(link_iNode));
	active[0]->next=NULL;

	active[1]=(link_iNode *)malloc(sizeof(link_iNode));
	active[1]->next=NULL;

	sys=(link_iNode *)malloc(sizeof(link_iNode));
	sys->next=NULL;

	state=0;
	while(!state) 
	{
		state=logon();
	}

	string operation;
	while(state)
	{
		state=execute();
	}

	if(fwrite(p,1,512*512,fp)==0) cout<<"write file wrong!";
	free(p);
}