#include "20161665.h"

/*******************************************************************
* 20161665 컴퓨터 공학과 황세현									   *
*																   *
* 어셈블러, 링크, 로더들을 실행하게 될 shell과 가상의 메모리공간   *
* 그리고 opcode mnemonic관련 명령어를 구현하는 프로그램입니다.	   *
*															       *
* shell에 make를 입력하면 실행파일(20161665.out)이 생성됩니다. 	   *	
********************************************************************/

static int Pow16[5]={1,16,256,4096,65536};
hashNode* hashTable[20]={0};

int main(){
	char tmpOrder[81];			//처음 입력되는 명령어 덩어리
	char order[30];				//입력받은 명령어
	char memory[66536][16];		//1MB의 메모리할당
	int valid=False; 			//명령어의 유효성을 나타내는 변수
	int lastAddr=0,lastLine=0;
	int a=-1,b=-1,c=-1;			//각각의 변수에  주소 또는 value가 저장됨
	int col,row;

	Queue h_queue;				//history 기능에 쓰일 자료구조

	char mnemo[8];

	/* initialization start */
	init_queue(&h_queue);
	init_memory(memory);
	create_hashTable();
	/* initialization end */

	while(1){
		valid=False;			//valid 값 초기화 

		/* 문자열 입력 받기 */
		printf("sicsim> ");
		fgets(tmpOrder,sizeof(tmpOrder),stdin);
		tmpOrder[strlen(tmpOrder)-1]='\0';
		/* 입력받은 문자열을 분리 */
		divide_str(tmpOrder,order,&a,&b,&c,mnemo);
		
		/* 종료 조건 */
		if(!strcmp(order,"q\0") || !strcmp(order,"quit\0")){
			func_quit(&h_queue);
			break;
		}
		/*******************명령어 처리 시작!********************/
		if(!strcmp(order,"h\0") || !strcmp(order,"help\0")){
			func_help();		//help function 기능 수행
			valid=True;			//유효한 명령어라면 valid=True
		}
		else if(!strcmp(order,"d\0") || !strcmp(order,"dir\0")){
			func_dir();
			valid=True;
			}
		else if(!strcmp(order,"hi\0") || !strcmp(order,"history\0")){
			if(h_queue.num==0)
				add_Node_To_Queue(tmpOrder,&h_queue);
			else{
				add_Node_To_Queue(tmpOrder,&h_queue);
				func_history(&h_queue);
			}
		}
		else if(!strcmp(order,"du\0") || !strcmp(order,"dump\0")){
			func_dump(memory,a,b,&lastAddr,&lastLine);
			a=b=c=-1;			//사용 후 주소, value 값 초기화
			valid=True;
		}
		else if(!strcmp(order,"reset\0")){
			init_memory(memory);
			valid=True;
		}
		else if(!strcmp(order,"edit\0") || !strcmp(order,"e\0")){
			col=a/16;row=a%16;
			memory[col][row]=b;
			a=b=c=-1;			
			valid=True;
		}
		else if(!strcmp(order,"fill\0") || !strcmp(order,"f\0")){
			func_fill(memory,a,b,c);
			a=b=c=-1;
			valid=True;
		}
		else if(!strcmp(order,"opcodelist\0")){
			func_opcodelist();
			valid=True;
		}
		else if(!strcmp(order,"opcode\0")){
			func_opcode_mnemonic(mnemo);
			valid=True;
		}
		else{					//에러코드 정리(invalid 명령어들)
			if(!strcmp(order,"incorrect\0"))
				printf("[ERROR]Invalid command\n");
			else if(!strcmp(order,"incorrectAddr\0"))
				printf("[ERROR]Invalid address\n");
			else if(!strcmp(order,"incorrectVal\0"))
				printf("[ERROR]Invalid value\n");
		}
		/********************명령어 처리 종료!*******************/
		
		if(valid==True)			//유효한 명령어라면 history에 저장한다.
			add_Node_To_Queue(tmpOrder,&h_queue);

		}

	return 0;
}

/* 메모리 공간을 초기화 하는 함수 */
void init_memory(char m[][16]){ 
	int i,j;

	for(i=0;i<66536;i++){
		for(j=0;j<16;j++){
			m[i][j]=0;
		}
	}
}
/* 어떤 수를 16진수로 변환할 경우 몇자리 수인지 */
int dec_to_hexa_digit(int a){
	if(a<0)
		return -1;
	else if(a<Pow16[1])
		return 1;
	else if(a<Pow16[2])
		return 2;
	else if(a<Pow16[3])
		return 3;
	else if (a<Pow16[4])
		return 4;
	else
		return 5;
}
/* dump 기능을 처리하는 함수 
a와 b에는 주소정보가 담겨있다. */
void func_dump(char memory[][16],int a,int b,int *lastAddr,int *lastLine){

	int i,j,tmp,return_to_start=False;
	int start=a,end=b;

	if(start>end && end!=-1){					//start>end일경우 예외처리
		printf("[ERROR]Invalid address\n");
		return;
	}
	else{
		if(start!=-1){
			*lastLine=start/16;
		}
		/* start, end 주소값 보정 */
		if(start==-1)		//start값이 설정돼있지 않을 경우(dump 단독 입력시)
			start=*lastAddr;
		if(end==-1){		//end값이 설정돼있지 않을 경우 (dump 단독 or dump start)
			end=start+159;
		}

		/* 메모리 한줄씩 출력 시작 */
		for(i=*lastLine;!return_to_start;i++){
			/* 1. memory 주소 출력 부분*/
			switch(dec_to_hexa_digit(i)){
				case 1 : printf("000%X0 ",i);break;
				case 2 : printf("00%X0 ",i);break;
				case 3 : printf("0%X0 ",i);break;
				case 4 : printf("%X0 ",i);break;
				case 5 : *lastLine=0;*lastAddr=0;return_to_start=True;
						continue;
				default : printf("[ERROR]Invalid memory access\n");break;
			}
			/* 2. 각 메모리에 저장된 값을 hexa로 출력 */		
			for(j=0;j<16;j++){
				if((tmp=i*16+j)>=start && tmp<=end){
					if((unsigned char)memory[i][j]<0x10)
						printf("0%X ",(unsigned char)memory[i][j]);
					else
						printf("%2X ",(unsigned char)memory[i][j]);
				}
				else
					printf("   ");
			}
			printf("; ");
			/* 3. 각 메모리에 저장된 값을 char로 출력 */
			for(j=0;j<16;j++){
				if((tmp=i*16+j)>=start && tmp<=end && memory[i][j]>=0x20 && memory[i][j]<=0x7E)
					printf("%c",memory[i][j]);
				else
					printf(".");
			}
			/* 반복문 탈출 조건*/
			tmp++;	
			if(tmp>end){ 
				if(end%16==0xF)
					(*lastLine)++;
				break;
			}
			
			printf("\n");
			(*lastLine)++;
		}
		/*dump 기능 종료 후  새로운 값으로 갱신*/
		if(!return_to_start){
			*lastAddr=end+1;	//주소값이 0xfffff를 초과하지 않았을 때만 갱신
			printf("\n");
		}
		if(*lastAddr==MAX_ADDR){
			return_to_start=True;
			*lastAddr=0;*lastLine=0;
		}
	}
}
/* dir 기능을 수행하는 함수 */
int func_dir(void){
	DIR *dir=opendir("./");
	struct dirent *tmp;
	struct stat state;
	
	if(dir!=NULL){
		while((tmp=readdir(dir))!=NULL){
			if(!strcmp(tmp->d_name,".") || !strcmp(tmp->d_name,".."))
				continue;

			lstat(tmp->d_name,&state);

			if(S_ISDIR(state.st_mode))						//디렉토리의 경우
				printf("%s/  ",tmp->d_name);
			else if(S_ISREG(state.st_mode)){				
				if(state.st_mode & S_IXUSR)					//실행 파일의 경우
					printf("%s*  ",tmp->d_name);
				else										//나머지 파일의 경우
					printf("%s  ",tmp->d_name);
			}
		}
		printf("\n");
		closedir(dir);
	}
	else{
		perror("");
		return EXIT_FAILURE;
	}
	return 1;
}
/* history 노드를 추가하는 함수 */
void add_Node_To_Queue(char order[],Queue* h_queue){
	Node *tmpNode;

	tmpNode=(Node*)malloc(sizeof(Node));
	strcpy(tmpNode->h_order,order);
	tmpNode->next=NULL;

	if(h_queue->num==0){
		h_queue->head=h_queue->tail=tmpNode;
	}
	else{
		h_queue->tail->next=tmpNode;
		h_queue->tail=tmpNode;
	}
	h_queue->num++;

	return;
}
/* history 기능을 수행하는 함수 */ 
void func_history(Queue *h_queue){
	int i;
	Node* tmpNode;

	tmpNode=h_queue->head;
	i=1;
	while(1){
		printf("	%-3d   %s\n",i++,tmpNode->h_order);

		if(tmpNode==h_queue->tail)
			break;
		tmpNode=tmpNode->next;
		}

	return;
}
/* help 기능을 수행하는 함수 */
void func_help(void){
	printf("\nh[elp]\n");
	printf("d[ir]\n");
	printf("q[uit]\n");
	printf("hi[story]\n");
	printf("du[mp] [start, value]\n");
	printf("e[dit] address, value\n");
	printf("f[ill] start, end, value\n");
	printf("reset\n");
	printf("opcode mnemonic\n");
	printf("opcodelist\n\n");
	
	return;
}
/* queue를 초기화 하는 함수 */
void init_queue(Queue *a){
	a->head=NULL;
	a->tail=NULL;
	a->num=0;

	return;
}
/* 비슷한 명령어별로 그룹짓는 함수 */
int find_group(char order[]){
	if(!strcmp(order,"h\0") || !strcmp(order,"help\0"))
		return 1;
	else if(!strcmp(order,"d\0") || !strcmp(order,"dir\0"))
		return 1;
	else if(!strcmp(order,"q\0") || !strcmp(order,"quit\0"))
		return 1;
	else if(!strcmp(order,"hi\0") || !strcmp(order,"history\0"))
		return 1;
	else if(!strcmp(order,"reset\0"))
		return 1;
	else if(!strcmp(order,"opcodelist\0"))
		return 1;
	else if(!strcmp(order,"du\0") || !strcmp(order,"dump\0"))
		return 3;
	else if(!strcmp(order,"e\0") || !strcmp(order,"edit\0"))
		return 4;
	else if(!strcmp(order,"f\0") || !strcmp(order,"fill\0"))
		return 5;
	else if(!strcmp(order,"opcode\0"))
		return 2;
	else
		return -1;
}
/* 초기의 입력받은 문자열에서 명령어, 주소등을 분리해내는 함수 
잘못된 문자열일 경우 True반환, 그렇지 않으면 False 반환 */
int get_string(char* origin,char* order, int *start,int comma_num){
	int i;
	int no_string=True;
	int valid=0;
	int count=0;

	for(i=*start;i<=strlen(origin);i++){
		if(origin[i]==' ' || origin[i]=='\t' || origin[i]=='\0' || origin[i]==','){
			if(origin[i]==',')
				count++;
			if(count>=2) return True;
		}
		else{
			no_string=False;
			order[valid++]=origin[i];
			if(comma_num!=count) return True;
			if(origin[i+1]==' ' || origin[i+1]=='\t' || origin[i+1]=='\0' || origin[i+1]==','){
				order[valid]='\0';
				break;
			}
		}
	}
	*start=i+1;

	return no_string;
}
/* 문자열이 정상적으로 종료되는지 검사하는 함수 
정상 종료시 1반환, 그렇지 않을 경우 0반환*/
int test_tail(char *origin,int start){
	int i;

	for(i=start;i<=strlen(origin);i++){
		if(origin[i]==' ' || origin[i]=='\t' || origin[i]=='\0')
			continue;
		else
			return 0;
	}
	return 1;
}
/* 16진수 표현의 문자열을 정수로 바꾸는 함수
flag=1 이면 주소변환, flag=0이면 value 변환
정상종료시 주소 또는 value 반환, 그렇지 않을 경우 -1반환*/
int string_to_number(char *tmp,int flag){
	int len=strlen(tmp);
	int i,sum=0;

	if(flag){
		if(len>5) return -1;
	}
	else{
		if(len>2) return -1;
	}
	
	for(i=0;i<len;i++){
		if(IS_VALID(tmp[i]))
			sum += correct_Addr(tmp[i])*Pow16[len-1-i];
		else
			return -1;
	}
	return sum;
}
/* 처음 입력받은 문자열에서 명령어, 주소, value, mnemonic 등을 분리하고
이를 각각의 속성에 알맞게 변환하는 함수. 올바르지 않은 문자열일 때는 
문자열 부분에 에러코드를 저장한다. */
void divide_str(char* origin,char* order,int *a,int *b,int *c,char *mnemonic){
	int start=0;
	char tmp[20];

	/* 문자열 덩어리에서 명령어를 뽑아냄 */
	if(get_string(origin,order,&start,0)){
		strcpy(order,"incorrect\0");
		return;
	}
	/* 어떠한 명령어인지 식별하는 과정 */
	switch(find_group(order)){
		case 1 : //case 1 : 명령어만 단독으로 존재하는 것(no address,value)
			if(!test_tail(origin,start))
				strcpy(order,"incorrect\0");
			return;
		case 2 : //case 2 : opcode mnemonic
			if(get_string(origin,tmp,&start,0)){	//mnemonic문자열 get
				strcpy(order,"incorrect\0");
				return;
			}
			if(!test_tail(origin,start)){
				strcpy(order,"incorrect\0");return;
			}
			strcpy(mnemonic,tmp);						//mnemonic 문자열 전달

		case 3 : //case 3 : dump 명령어
			if(test_tail(origin,start)) return;			//dump 명령어만 입력된경우

			if(get_string(origin,tmp,&start,0)){
				strcpy(order,"incorrect\0");return;		//오류 처리ex) dump 	,
			}

			if((*a=string_to_number(tmp,1))==-1){		//첫번째 주소 저장
				strcpy(order,"incorrectAddr\0");return;	//올바르지 못한 주소 에러처리
			}
			
			if(test_tail(origin,start)) return;			//dump start 명령어 정상처리

			if(get_string(origin,tmp,&start,1)){
				strcpy(order,"incorrect\0");return;		//오류 처리ex) dump B2 ,
			}

			if((*b=string_to_number(tmp,1))==-1){		//두번째  주소 저장
				strcpy(order,"incorrectAddr\0");return;	//올바르지 못한 주소 에러처리
			}

			if(test_tail(origin,start)) return;

		case 4 : //case 4 : edit 명령어
			if(test_tail(origin,start) || get_string(origin,tmp,&start,0)){
				strcpy(order,"incorrect\0");return;
			}
			if((*a=string_to_number(tmp,1))==-1){			//첫번째 주소 저장
				strcpy(order,"incorrectAddr\0");return;		//올바르지 못한 주소 에러처리
			}
			if(test_tail(origin,start) || get_string(origin,tmp,&start,1)){
				printf("3\n");
				strcpy(order,"incorrect\0");return;
			}
			if((*b=string_to_number(tmp,0))==-1){			//두번째 value 저장
				strcpy(order,"incorrectVal\0");return;		//올바르지 못한 value 에러처리
			}
			if(!test_tail(origin,start))
				strcpy(order,"incorrect\0");
			break;

		case 5 : //case 5 : fill 명령
			if(test_tail(origin,start) || get_string(origin,tmp,&start,0)){
				strcpy(order,"incorrect\0");return;
			}
			if((*a=string_to_number(tmp,1))==-1){			//첫번째 주소 저장
				strcpy(order,"incorrectAddr\0");return;		//올바르지 못한 주소 에러처리
			}
			if(test_tail(origin,start) || get_string(origin,tmp,&start,1)){
				strcpy(order,"incorrect\0");return;
			}
			if((*b=string_to_number(tmp,1))==-1){			//두번째 주소  저장
				strcpy(order,"incorrectAddr\0");return;		//올바르지 못한 주소  에러처리
			}
			if(test_tail(origin,start) || get_string(origin,tmp,&start,1)){
				strcpy(order,"incorrect\0");return;
			}
			if((*c=string_to_number(tmp,0))==-1){			//세번째 value  저장
				strcpy(order,"incorrectVal\0");return;		//올바르지 못한 value 에러처리
			}
			if(!test_tail(origin,start))
			 strcpy(order,"incorrect\0");
			 break; 
		default : strcpy(order,"incorrect\0");return;
	}

}
/* 입력된 문자가 16진수 범위 내에 있는지 검사하는 함수 */
int IS_VALID(char a){
	if((a>='0' && a<='9')||(a>='A' && a<='F')||(a>='a' && a<='f'))
		return True;
	return False;
}
/* 입력된 문자가 숫자인지 검사하는 함수 */
int IS_NUMBER(char a){
	if(a>='0' && a<='9')
		return True;
	return False;
}
/* 입력된 문자가 알파벳 대문자인지 검사하는 함수 */
int IS_UPPER(char a){
	if(a>='A' && a<='F')
		return True;
	return False;
}
/* 전달된 문자를 알맞은 16진수 값으로 보정하는 함수 */
int correct_Addr(char a){
	if(IS_NUMBER(a))
		return a-'0';
	else if(IS_UPPER(a))
		return a-'A'+10;
	else
		return a-'a'+10;
}
/* fill 명령어 기능을 수행하는 함수 */
void func_fill(char memory[][16],int start,int end,int val){
	int i,j,tmp;

	for(i=start/16;i<=end/16;i++){
		for(j=0;j<16;j++){
			if((tmp=i*16+j)>=start && tmp<=end)
				memory[i][j]=val;
		}
	}
}
/* 해쉬함수 : mnemonic에 대한 index반환 */
int func_hash(char *mnemonic){
	int len=strlen(mnemonic);
	int sum=0,i;
	for(i=0;i<len;i++)
		sum += mnemonic[i];
	return sum%20;
}
/* 프로그램 초반에 hashTable을 생성하는 함수 */
void create_hashTable(){
	FILE* fp=fopen("opcode.txt","r");
	char str[8],tmp[5];					
	unsigned int num;
	int idx;
	hashNode *newNode,*tmpPtr;

	while(fscanf(fp,"%x %s %s",&num,str,tmp)!=EOF){
		idx=func_hash(str);tmpPtr=hashTable[idx];
		newNode=(hashNode*)malloc(sizeof(hashNode));
		strcpy(newNode->mnemonic,str);
		newNode->hexa=num;
		newNode->next=NULL;
		
		if(tmpPtr==NULL)
			hashTable[idx]=newNode;
		else{
			while(tmpPtr->next!=NULL)
				tmpPtr=tmpPtr->next;
			tmpPtr->next=newNode;
		}
	}	

	fclose(fp);
}
/* 형성된 hashTable을 바탕으로 opcodelist를 출력하는 함수 */
void func_opcodelist(){
	int i;
	hashNode* tmp;

	printf("\n");
	for(i=0;i<20;i++){
		tmp=hashTable[i];
		printf("%-2d : ",i);

		if(tmp!=NULL){
			while(tmp!=NULL){
				if(tmp->hexa<16)
					printf("[%s,0%X] ",tmp->mnemonic,tmp->hexa);
				else
					printf("[%s,%2X] ",tmp->mnemonic,tmp->hexa);
				if(tmp->next!=NULL)
					printf("-> ");
				tmp=tmp->next;
			}
		}
	
		printf("\n");
	}
	printf("\n");
}
/* opcode mnemonic이 입력될 경우 해당하는 opcode를 출력 */
void func_opcode_mnemonic(char *mnemonic){
	int idx=func_hash(mnemonic);
	int find=False;

	hashNode* tmp=hashTable[idx];

	while(tmp!=NULL){
		if(!strcmp(tmp->mnemonic,mnemonic)){
			find=True;
			if(tmp->hexa<16)
				printf("opcode is 0%X\n",tmp->hexa);
			else
				printf("opcode is %2X\n",tmp->hexa);
		}
		tmp=tmp->next;
	}
	if(!find)
		printf("[ERROR]Invalid mnemonic\n");
}
/* 동적으로 할당된 메모리를 free하는 함수 */
void func_quit(Queue *q){
	Node* del;
	hashNode* del2;
	int i;

	/* 1. history목록 free */
	if(q->num>0){
		del=q->head;
		if(q->num==1)
			free(del);
		else{
			while(q->head){
				del=q->head;
				q->head=q->head->next;
				free(del);
			}
		}
		q->num=0;
	}

	/* 2. hashTable free */
	
	for(i=0;i<20;i++){
		if(hashTable[i]){
			while(hashTable[i]){
				del2=hashTable[i];
				hashTable[i]=hashTable[i]->next;
				free(del2);
			}
		}
	}

	return;
}

