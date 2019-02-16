#include "20161665.h"

/*******************************************************************
* 20161665 컴퓨터 공학과 황세현									   *
*																   *
* 1. 어셈블러, 링크, 로더들을 실행하게 될 shell과 가상의 메모리공간*
* 그리고 opcode mnemonic관련 명령어를 구현하는 프로그램입니다.	   *
* 2. SIC/XE assembler 를 구현한 프로그램 입니다. 				   *
*															       *
* shell에 make를 입력하면 실행파일(20161665.out)이 생성됩니다. 	   *	
********************************************************************/

static int Pow16[6]={1,16,256,4096,65536,1048576};
hashNode* hashTable[20]={0};
symbolNode* symbolTable[26]={0};
modHead modi={0,NULL};
int assemble_num=0;		//어셈블이 실행된 횟수(성공했을때)를 기록
int lastAssemble=False; //직전 어셈블링의 성공여부
int prog_length=0;		//.obj코드에 적힐 프로그램 길이

////////////주의사항//////////////////
//filename 최대 30자				//
//어셈파일 길이 최대 120자 로 받음	//
//BYTE X최대 3bytes hexa가능		//
//BYTE C최대 20bytes 문자열 가능	//
//////////////////////////////////////

int main(){
	char tmpOrder[81];			//처음 입력되는 명령어 덩어리
	char order[30];				//입력받은 명령어
	char memory[66536][16];		//1MB의 메모리할당
	int valid=False; 			//명령어의 유효성을 나타내는 변수
	int lastAddr=0,lastLine=0;
	int a=-1,b=-1,c=-1;			//각각의 변수에  주소 또는 value가 저장됨
	int col,row;
	char extra[30];				//opcode또는 filename을 담을 변수
	char temp;

	Queue h_queue;				//history 기능에 쓰일 자료구조

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
		divide_str(tmpOrder,order,&a,&b,&c,extra);
		
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
			func_opcode_mnemonic(extra,1,&temp);
			valid=True;
		}
		else if(!strcmp(order,"type\0")){
			if(func_type(extra)==1)
				valid=True;
		}
		else if(!strcmp(order,"assemble\0")){
			if(assemble_num>=1 && lastAssemble)
				delete_symbolTable();		//기존에 형성된 symbolTable삭제 후 시작
			if(pass1(extra)==1){			//pass1 정상종료
				if(pass2(extra)==1){		//pass2 정상종료
					lastAssemble=True;
					assemble_num++;
					valid=True; 			//pass2까지 정상종료했을때만 valid=True;
				}
				else{						//pass2 비정상종료
					delete_symbolTable();
					lastAssemble=False;
				}
			}	
			else{							//pass1 비정상종료
				delete_symbolTable();
				lastAssemble=False;
			}
		}
		else if(!strcmp(order,"symbol\0")){
			func_symbol();
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
	printf("opcodelist\n");
	printf("assemble filename\n");
	printf("type filename\n");
	printf("symbol\n\n");
	
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
	else if(!strcmp(order,"symbol\0"))
		return 1;
	else if(!strcmp(order,"du\0") || !strcmp(order,"dump\0"))
		return 3;
	else if(!strcmp(order,"e\0") || !strcmp(order,"edit\0"))
		return 4;
	else if(!strcmp(order,"f\0") || !strcmp(order,"fill\0"))
		return 5;
	else if(!strcmp(order,"opcode\0") || !strcmp(order,"type\0") || !strcmp(order,"assemble\0"))
		return 2;
	else
		return -1;
}
/* 초기의 입력받은 문자열에서 명령어, 주소등을 분리해내는 함수 
잘못된 문자열일 경우 True반환, 그렇지 않으면 False 반환 
추가) comma_num==100일때는 directive 문자 분리역할 */
int get_string(char* origin,char* order, int *start,int comma_num){
	int i;
	int no_string=True;
	int valid=0;
	int count=0;

if(comma_num<100){
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
				order[valid]='\0'; 			//문자열 끝부분에 NULL문자 삽입
				break;
			}
		}
	}
	*start=i+1;
}
	else{			//assembler directive 문자열 처리과정(작은 따옴표 생략 후 저장)
		count=0;i=*start;valid=0;	//작은 따옴표의 개수를 count
		while(count!=2 && origin[i]!='\0'){
			if(count==0){
				if(origin[i]==' ' || origin[i]=='\t'){
					i++;
					continue;
				}
				else if(origin[i]=='\''){		//작은 따옴표일경우
					count++;i++;
					continue;
				}
				else
					order[valid++]=origin[i++];
			}
			else if(count==1){
				if(origin[i]!='\'')
					order[valid++]=origin[i++];
				else
					count++;
			}
		}
		if(count==2){
			order[valid]='\0';
			no_string=False;
		}

		*start=i+1;	
	}
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
flag=2이면 6자리수까지 하용
정상종료시 주소 또는 value 반환, 그렇지 않을 경우 -1반환*/
int string_to_number(char *tmp,int flag){
	int len=strlen(tmp);
	int i,sum=0;

	if(flag==1){
		if(len>5) return -1;
	}
	else if(!flag){
		if(len>2) return -1;
	}
	else{
		if(len>6) return -1;

		for(i=0;i<len;i++){
			if(tmp[i]>='a' && tmp[i]<='f')
				return -1;
		}
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
void divide_str(char* origin,char* order,int *a,int *b,int *c,char *extra){
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
		case 2 : //case 2 : opcode mnemonic 또는 type 또는 assemble
			if(get_string(origin,tmp,&start,0)){	//extra 문자열 get
				strcpy(order,"incorrect\0");
				return;
			}
			if(!test_tail(origin,start)){
				strcpy(order,"incorrect\0");return;
			}
			strcpy(extra,tmp);						//extra 문자열 전달

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
		strcpy(newNode->option,tmp);
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
unsigned int func_opcode_mnemonic(char *mnemonic,int option,char *a){
	int idx=func_hash(mnemonic);
	int find=False;

	hashNode* tmp=hashTable[idx];

	while(tmp!=NULL){
		if(!strcmp(tmp->mnemonic,mnemonic)){
			find=True;
			if(option==1){
				if(tmp->hexa<16)
					printf("opcode is 0%X\n",tmp->hexa);
				else
					printf("opcode is %2X\n",tmp->hexa);
			}
			*a=(tmp->option[0]);
			return tmp->hexa;
		}
		tmp=tmp->next;
	}
	if(!find && option)
		printf("[ERROR]Invalid mnemonic\n");
	return 0xFFFF;
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
int func_type(char *filename)
{
	FILE *fp;
	char tmp;

	if(!(fp=fopen(filename,"r"))){		//file이 존재하지 않을경우
		printf("[ERROR]No files having such a name [%s]\n",filename);
		return -1;						//-1값을 return한다.
	}
	
	while((tmp=fgetc(fp))!=EOF)
		printf("%c",tmp);

	fclose(fp);
	return 1;
}
/* 에러 발생시 -1반환, 정상 종료시 1반환 */
int pass1(char *filename){
	FILE *fp_read,*fp_write;
	char sentence[100],order[30],name[20],temp[20],symbol[20];
	int start=0,locctr=0,err_line=-1,cur_line=0;
	int no_start=False,res,symbol_loc;    //첫째줄에 start directive의 존재유무 판단 flag
	int prog_start,i=0;				      //program start 주소
	int blank=0,len,can_start=0;
	ERROR error;						  //에러코드를 담을 변수
	what_type type;

	if(!(fp_read=fopen(filename,"r"))){   //파일 열기 에러
		printf("[ERROR]No files having such a name [%s]\n",filename);
		return -1;
	}
	fp_write=fopen("pass1.txt","w");
	
	/************** 1. .asm 파일 첫번째줄 읽어들이기 **************/
	while(!can_start){										//공백줄이 아닐때 까지 반복
		fgets(sentence,sizeof(sentence),fp_read);
		sentence[strlen(sentence)-1]='\0';
	
	/*  첫째줄이 공백 줄인지 아닌지 검사 */
	len=strlen(sentence);i=0;
	for(i=0;i<len;i++){
		if(sentence[i] == ' ' || sentence[i]=='\t')
			continue;
		blank++;
	}
	if(!blank){ //첫째줄이 공백줄이라면 
		type=BLANK;
		fprintf(fp_write,"%X %d\n",locctr,type);
		continue;
	}
	
	/* 첫째줄 에러 검사 (공백 줄이 아닐 때)*/
	can_start=1;											//이제 시작할 수 있음
	if(get_string(sentence,order,&start,0)==1){				//program 이름 추출
		error=get;err_line=1;
	}
	if(get_string(sentence,temp,&start,0)==1){				//start address 추출
		error=get;err_line=1;
	}

	//program name & start address 설정
	if(!strcmp(temp,"START\0")){
		strcpy(name,order);
		if(get_string(sentence,order,&start,0)){
			error=get;err_line=cur_line;
		}
		else if(!test_tail(sentence,start)){
			error=tail;err_line=cur_line;
		}
		locctr=string_to_number(order,1);
		cur_line=1;
	}
	else{
		strcpy(name,"NONAME\0");
		no_start=True;
	}
	prog_start=locctr;					//locctr 처음 주소 저장
	fprintf(fp_write,"%X %s\n",locctr,name);
}

	//예외처리) 첫째줄 에러처리 후 종료
	if(err_line!=-1){
		if(error==get) printf("[ERROR]Invalid code (line 5)\n");
		else if(error==tail) printf("[ERROR]Too many parameters (line 5)\n");
		fclose(fp_write);fclose(fp_read);remove("pass1.txt");
		return -1;
	}
	
	/************ 2. END directive에 도달할때까지 어셈블링 과정 반복***********/
	while(1){							//error line발견시 또는 END 도달시 prog 종료
			if(!no_start)
				start=0;
			symbol_loc=locctr;
			fprintf(fp_write,"%X ",locctr);

	/* a)문장의 첫번째 order 처리하기 : symbol 또는 mnemonic 또는 END*/
			if(!no_start)				//start directive가 없으면 뛰어넘기(딱 1번)
			{
				fgets(sentence,sizeof(sentence),fp_read);
				sentence[strlen(sentence)-1]='\0';
				 
				len=strlen(sentence);i=0;blank=0;
				for(i=0;i<len;i++){
					if(sentence[i] == ' ' || sentence[i]=='\t')
						continue;
					blank++;
				}

				if(!blank){ //공백줄이라면 
					type=BLANK;
					fprintf(fp_write,"%d\n",type);
					continue;
				}

				if(get_string(sentence,order,&start,0)){
					err_line=cur_line;error=get;break;
				}
			}
			cur_line++;

			if(!strcmp(order,"END\0")){			//종료 조건(type 4)
				fprintf(fp_write,"4\n");
				prog_length=locctr-prog_start;  //prog 길이 저장(.obj에 쓰임)
				break;
			}
			else if(!strcmp(order,"BASE\0")){	//base directive (type 10)
				fprintf(fp_write,"10 ");
			 //나머지 뒤에있는 문자열 그대로 저장	
				for(i=start;sentence[i]!='\0';i++)
					fprintf(fp_write,"%c",sentence[i]);
				fprintf(fp_write,"\n");

				continue;
			}
			else if(!strcmp(order,"NOBASE\0")){	//nobase directive (type 11)
				fprintf(fp_write,"11\n");
				continue;
			}
			else if(order[0]=='.'){				//comment line (type 9)
				fprintf(fp_write,"9\n");
				continue;
			}
			//mnemonic인지 아닌지를 검사
			if(if_mnemonic(fp_write,sentence,order,&locctr,start)==1){	//뉴모닉이라면
				no_start=False;
				continue;
			}
			strcpy(symbol,order);				//임시로 symbol에 저장

	/* b)문장의 두번째 order 처리하기(여기서 symbol인지 아닌지를 판별)*/
			if(!no_start){
				if(get_string(sentence,order,&start,0)){ //문자열 1개더 읽음
					err_line=cur_line;error=get;break;
				}
			}
			else
				strcpy(order,temp);
		    //mnemonic인지 아닌지를 검사
			if(if_mnemonic(fp_write,sentence,order,&locctr,start)==1){	//뉴모닉이라면
				no_start=False;
			}
			//에러가 발생할 경우
			else if((res=if_directive(fp_write,sentence,order,&locctr,&start))!=1){
				err_line=cur_line;error=directive;break;
			}

	/* c)이 모든 과정을 거쳐왔다면 처음의 order은 symbol이다. */
			add_node_to_symTable(symbol,symbol_loc);

			no_start=False;
		}
		/* 에러코드 처리 */
		if(err_line!=-1){				//에러가 발생했을 때
			if(error==get)
				printf("[ERROR]Invalid string ");
			else if(error==tail)
				printf("[ERROR]Too many parameters ");
			else if(error==directive)
				printf("[ERROR]Invalid directive ");
			printf("(line %d)\n",err_line*5);
			fclose(fp_read);fclose(fp_write);remove("pass1.txt");
			return -1;
		}

	fclose(fp_read);fclose(fp_write);	//만약 중간에 에러발생시 pass1.txt지워주기
	return 1;
}
/* directive일경우 1, directive 아닐 경우 -1, error line일 경우 0반환 */
int if_directive(FILE*fp_write,char *origin,char *order,int *locctr,int *start){
	what_type type;
	int num,i,len;
	int hexa_flag=0;

	if(!strcmp(order,"BYTE\0")){						//BYTE directive
		type=BYTE;
		
		if(get_string(origin,order,start,100)){
			return 0;
		}
		else if(!test_tail(origin,*start))				//이외의 문자열이 더 있을때
			return 0;
		//C'E   OF'같은 경우 제대로 안받아짐
		//대신 hexa는 띄어쓰기 중간에 있으면 안됨
		len=strlen(order);

		if(order[0]=='C' || order[0]=='X'){
			if(order[0]=='X') hexa_flag=1;
			for(i=1;i<len;i++)
				order[i-1]=order[i];
			order[len-1]='\0';

			len=strlen(order);

			fprintf(fp_write,"%d ",type);
			if(hexa_flag)								//X라면(hexa범위 벗어날때 error처리 필요)
			{
				if((num=string_to_number(order,2))==-1){
					printf("[ERROR]Invalid hexa value \n");
					return 0;
				}
				if(len%2)
					*locctr += (len+1)/2;
				else
					*locctr += len/2;

				fprintf(fp_write,"X %X\n",num);
			}
			else										//C라면
			{
				fprintf(fp_write,"C ");
				for(i=0;i<len;i++)
					fprintf(fp_write,"%X",order[i]);
				fprintf(fp_write,"\n");
				*locctr += len;
			}
		}		
		else
			return 0; 									//에러발생
	}
	else if(!strcmp(order,"WORD\0")){					//WORD directive
		type=WORD;
		get_string(origin,order,start,0);
		num=string_to_number2(order);

		if(num>0xFFFFFF)
			return 0;									//에러발생시 0을 반환
		else if(!test_tail(origin,*start)) 				//숫자 뒤에 문자열이 더 존재할 경우
			return 0;

		*locctr += 3;
		fprintf(fp_write,"%d %X\n",type,num);
	}
	else if(!strcmp(order,"RESB\0")){					//RESB directive
		type=RESB;
		get_string(origin,order,start,0);
		num=string_to_number2(order);

		if(*locctr+num>0xFFFF)							//에러처리
			return 0;
		else if(!test_tail(origin,*start))				//에러처리
			return 0;

		*locctr += num;
		fprintf(fp_write,"%d\n",type);
	}
	else if(!strcmp(order,"RESW\0")){					//RESW directive
		type=RESW;
		get_string(origin,order,start,0);
		num=string_to_number2(order);

		if(*locctr+num*3>0xFFFF)
			return 0;
		else if(!test_tail(origin,*start))
			return 0;
		*locctr += num*3;
		fprintf(fp_write,"%d\n",type);
	}
	else												//directive가 아닌경우 -1 반환
		return -1;
	
	return 1;
}
int if_mnemonic(FILE *fp_write,char *origin,char *mnemo,int* locctr,int start){
	char format;
	unsigned int opcode=0xFFF;
	int plus=False,len=strlen(mnemo),i;

	if(mnemo[0]=='+'){
		plus=True;								//+mnemonic의 경우

		for(i=1;i<len;i++)
			mnemo[i-1]=mnemo[i];
		mnemo[len-1]='\0';
	}

	opcode=func_opcode_mnemonic(mnemo,0,&format);

	if(opcode>(unsigned int)0xFF) 				//mnemonic이 아닌 경우
		return -1;

	if(plus){									//mnemonic이 맞는 경우
		*locctr += 4;
		fprintf(fp_write,"%d %X",8,opcode);
	}
	else{
		i=format-'0';
		if(i==1)
			*locctr += 1;
		else if(i==2)
			*locctr += 2;
		else if(i==3)
			*locctr += 3;
		fprintf(fp_write,"%d %X ",format-'1'+5,opcode); //type과 mnemonic 기록
	}
 	//나머지 뒤에있는 문자열 그대로 저장	
	for(i=start;origin[i]!='\0';i++)
		fprintf(fp_write,"%c",origin[i]);
	fprintf(fp_write,"\n");

	return 1;
}
/* 10진수 문자열을 10진수 정수로 바꾸는 함수 */
int string_to_number2(char *tmp){
	int len=strlen(tmp);
	int i,sum=0;

	for(i=0;i<len;i++)
		sum += ((tmp[i]-'0')*Pow10(len-(i+1)));
	return sum;
}
int Pow10(int a){
	if(a<=0)
		return 1;
	else
		return 10*Pow10(a-1);
}
void add_node_to_symTable(char symbol[20],int addr){
	int idx=symbol[0]-'A';
	symbolNode* tmp,*tmp2=NULL;
	/* 새로운 노드 생성 */
	symbolNode *newNode=(symbolNode*)malloc(sizeof(symbolNode));
	strcpy(newNode->symbol,symbol);
	newNode->line_addr=addr;
	newNode->next=NULL;
	
	if(!symbolTable[idx])						//노드가 없을때
		symbolTable[idx]=newNode;
	else{									//2개 이상의 노드가 존재할때
		tmp=symbolTable[idx];
		while(tmp!=NULL){
			if(strcmp(tmp->symbol,symbol)<0){
				if(tmp2==NULL){
					symbolTable[idx]=newNode;
					newNode->next=tmp;
				}
				else{	
					newNode->next=tmp;
					tmp2->next=newNode;
				}
				break;
			}	
			tmp2=tmp;tmp=tmp->next;
		}
		if(tmp==NULL)		
			tmp2->next=newNode;
	}
}

void func_symbol()
{
	int i,digit;
	int count=0;
	symbolNode* tmp;

	for(i=25;i>=0;i--){
		tmp=symbolTable[i];

		if(tmp==NULL){
			count++;
			continue;
			}
		else{
			while(tmp!=NULL){
				digit=dec_to_hexa_digit(tmp->line_addr);
				switch (digit) {
					case 1 : printf("\t%s\t000%X\n",tmp->symbol,tmp->line_addr);break;
					case 2 : printf("\t%s\t00%X\n",tmp->symbol,tmp->line_addr);break;
					case 3 : printf("\t%s\t0%X\n",tmp->symbol,tmp->line_addr);break;
					case 4 : printf("\t%s\t%X\n",tmp->symbol,tmp->line_addr);break;
					default : break;
				}
				tmp=tmp->next;
			}
		}
	}
	if(count==26)	//symbolTable이 전부 비어있을 때
		printf("\n[ No existing symbol Table ] \n\n");
}
/* assemble 명령어가 입력될때 현재 존재하는 symtable을 삭제한다. */
void delete_symbolTable(){
	int i;
	symbolNode* del;

	for(i=0;i<26;i++){
		if(symbolTable[i]){		
			while(symbolTable[i]){
				del=symbolTable[i];
				symbolTable[i]=symbolTable[i]->next;
				free(del);		
			}
		}		
	}
}
/* pass1이 정상적으로 처리되었다는 전제하에 pass2시작 
비정상종료시 -1 반환(error발생) --> pass1.txt .obj .lst 모두 삭제해야함
정상종료시 1 반환 */
int pass2(char *filename){
	FILE* fp_origin=fopen(filename,"r");
	FILE* fp_pass1=fopen("pass1.txt","r");
	FILE*fp_lst,*fp_obj;
	char file1[30],file2[30],name[20];
	char sentence[120];							//*.asm 파일의 1문장씩 담아낼 공간
	char tmp[50],tmp2[50],tmp3[50];;		
	char objcode[61], objrecord[30][61];		//임시로 obj code를 담을 공간
	int len,cur_line=1,locctr,format=-1,err_line=-1;
	int valid=0,byte=0,objLoc=0;				//objLoc=obj에서 시작 주소 담음
	int nixbpe[6]={0};							//nixbpe값을 저장하는 배열

	int hexa,i,j,start=0,prog_start=0; 
	int flag=True,objflag=True;					//flag는 obj코드 적는 시기를 알려주는 변수
	int b=False,baseLoc;						//p bit를 검사하는 flag/BASE가 사용가능조사 flag
	int pc,disp,ni=0,xbpe=0;
	char c;

	what_type type=0;							//type check하는 enum형 변수
	modeNode *newNode,*tmpNode;					//modification record에 사용될 노드
	ERROR error;								//에러코드를 담을 변수

	strcpy(name,"12\0");						//공백줄을 제거하기 위함 
	
	/* .lst 와 .obj 파일 생성하는 과정*/
	len=strlen(filename);
	strncpy(file1,filename,len-3);strncpy(file2,filename,len-3);
	file1[len-3]='\0';file2[len-3]='\0';
	strcat(file1,"lst");strcat(file2,"obj");

	fp_lst=fopen(file1,"w");fp_obj=fopen(file2,"w");

	/****************** 시작 주소 & 프로그램 이름 받아오기 ****************/
	while(!strcmp(name,"12\0")){					//첫줄이 공백줄이 아닐때까지 반복
	fscanf(fp_pass1,"%x %s\n",&locctr,name);

	if(!strcmp(name,"12\0")) {
		fgets(sentence,sizeof(sentence),fp_origin);	//한줄 흡수(blank 일때)
		continue;
	}

	objLoc=locctr;prog_start=locctr;

	/* .lst .obj 첫번째 줄 작성 */
	//1. .lst 파일
	fprintf(fp_lst,"%3d\t",cur_line*5);
	if(!strcmp(name,"NONAME\0") && locctr==0){		//START directive 가 없었다면
		loc_write_to_file(fp_lst,locctr,4);
		fprintf(fp_lst,"\t%-6s        %X\n",name,locctr);
	}
	else{											//START directive 가 있었다면
		loc_write_to_file(fp_lst,locctr,4);
		fprintf(fp_lst,"\t%-6s START  %X\n",name,locctr);
		fgets(sentence,sizeof(sentence),fp_origin);//기존파일 첫째줄 제거
	}
	//2. .obj 파일
	fprintf(fp_obj,"H%-6s",name);
	loc_write_to_file(fp_obj,locctr,6);
	loc_write_to_file(fp_obj,prog_length,6);
	fprintf(fp_obj,"\n");

	cur_line++;
	}

	/*******************반복적으로 명령어 처리 시작(MAIN)*******************/
	while(type!=END && err_line==-1){		//END directive 가 나오기 전까지 반복
		//변수 초기화
		format=0;objcode[0]='\0';
		/* obj file첫 주소를 저장(col 2-7)*/
		if(objflag && type!=RESW && type!=RESB && type!=COMMENT && type!= BASE && type!=NOBASE){
			objLoc=locctr;
			objflag=False;
		}

		/* source code(.asm) 받아오기 */
		fgets(sentence,sizeof(sentence),fp_origin);
		sentence[strlen(sentence)-1]='\0';

	
		len=strlen(sentence);//띄어쓰기 예쁘게 조정
		for(i=len;i<30;i++)
			sentence[i]=' ';
		if(len<30)
			sentence[30]='\0';

		/* locctr 과 type 받아오기 */
		fscanf(fp_pass1,"%X %d",&locctr,&i);type=i;

		//.lst 파일에 기록하기전에 type ==BLANK 라면
		if(type==BLANK) continue;	//넘어가기
		
		//1. 먼저 lst파일에 기록
		fprintf(fp_lst,"%3d\t",cur_line*5);
		if(type==BASE || type==NOBASE || type==COMMENT || type==END){
			fprintf(fp_lst,"   \t    \t");
			fprintf(fp_lst,"%s\n",sentence);
		}
		else{
			loc_write_to_file(fp_lst,locctr,4);
			fprintf(fp_lst,"\t%s\t\t",sentence);		//source code까지 기록
		}
		/* type 별로 각각 처리하기 */
		switch(type) {
			case BYTE : fscanf(fp_pass1,"%s",tmp);
						fscanf(fp_pass1,"%s",objcode);
						if(!strcmp(tmp,"X\0")){			//hexa일때
							if(strlen(objcode)%2==1) 		
							{	len=strlen(objcode);
								memcpy(objcode+1,objcode,strlen(objcode));
								objcode[0]='0';objcode[len+1]='\0';
								}
							format=1;
						}
						else							//문자열 범위도 지정되어있다.
							format=strlen(objcode)/2;
						fprintf(fp_lst,"%s\n",objcode);
						break;

			case WORD : fscanf(fp_pass1,"%s",tmp);
						len=strlen(tmp);
						objcode[0]='0';objcode[1]='0';objcode[2]='0';
						objcode[3]='0';objcode[4]='0';objcode[5]='0';objcode[6]='\0';
						memcpy(objcode+(6-len),tmp,len);
						fprintf(fp_lst,"%s\n",objcode);
						format=3;
						break;

			case RESB : fprintf(fp_lst,"\n");
						break;
			case RESW : fprintf(fp_lst,"\n");
						break;
			case END  : //할거 없음
						break;
			case OP1  : fscanf(fp_pass1,"%s",objcode);i=0;
						while((c=fgetc(fp_pass1))!='\n')
							tmp[i++]=c;					//tmp는 최대 50자.
						tmp[i]='\0';
						if(test_tail(tmp,0)!=1){		//에러처리
							err_line=cur_line;error=tail;
							break;
						}
						fprintf(fp_lst,"%s\n",objcode);
						format=1
						;break;
			case OP2  : fscanf(fp_pass1,"%s",objcode);i=0;	//opcode저장
						while((c=fgetc(fp_pass1))!='\n')	//뒤에 있는 문자열 추출
							tmp[i++]=c;
						tmp[i]='\0';start=0;
						/* reg개수 1개, 2개로 나뉨 */
						if(!strcmp(objcode,"B4\0") || !strcmp(objcode,"B8\0")){
							if(get_string(tmp,tmp2,&start,0)==1){
								err_line=cur_line;error=get;
							}
							else if(!test_tail(tmp,start)){
								err_line=cur_line;error=tail;
							}
							if(OP2_choose_reg(tmp2,objcode)!=1){
								printf("[ERROR]Invalid register %s ",tmp2);
								err_line=cur_line;break;
							}
							objcode[3]='0';
						}
						else{
							if(get_string(tmp,tmp2,&start,0)==1){		//첫번째 reg
								err_line=cur_line;error=get;
							}
							else if(OP2_choose_reg(tmp2,objcode)!=1){
								printf("[ERROR]Invalid register %s ",tmp2);
								err_line=cur_line;break;
							}
							
							if(get_string(tmp,tmp2,&start,1)==1){
								err_line=cur_line;error=get;
							}
							else if(!test_tail(tmp,start)){
								err_line=cur_line;error=tail;
							}
							
							if(OP2_choose_reg(tmp2,objcode)!=1){		//reg범위 오류
								err_line=cur_line;
								printf("[ERROR]Invalid register ");break;
							}
						}
						if(err_line!=-1) break;
						objcode[4]='\0';format=2;
						fprintf(fp_lst,"%s\n",objcode);
						break;
			case OP3  : 
			case OP_PLUS :
						/* nixbpe 값 초기화 */
						for(i=0;i<6;i++){nixbpe[i]=0;}
						if(type==OP_PLUS){
							nixbpe[5]=1;format=4;
						}
						else
							format=3;
						/* opcode 및 문자열 추출 */
						fscanf(fp_pass1,"%s",objcode);i=0;	//opcode저장
						while((c=fgetc(fp_pass1))!='\n')	//뒤에 있는 문자열 추출(tmp)
							tmp[i++]=c;
						tmp[i]='\0';start=0;

						if(strlen(objcode)==1){				//opcode 1자리->2자리 보정
							objcode[2]=objcode[1];
							objcode[1]=objcode[0];
							objcode[0]='0';
						}
						if(!strcmp(objcode,"4C\0")){		//RSUB의 경우!
							if(!test_tail(tmp,start)){
								err_line=cur_line;error=tail;
							}
							else if(type==OP3)
								strcpy(objcode,"4F0000\0");
							else if(type==OP_PLUS)
								strcpy(objcode,"4F000000\0");
						}	
						else{			//나머지 3/4format 처리
							if(get_string(tmp,tmp2,&start,0)==1){	//첫번째 문자열추출
								err_line=cur_line;error=get;		//에러코드
							}

							//0.  n,i bit 설정하기 
							if(tmp2[0]=='@' || tmp2[0]=='#'){
								if(tmp2[0]=='@')nixbpe[0]=1;  		//indirect
								else if(tmp2[0]=='#')nixbpe[1]=1;	//immediate

								//flag를 set 했으므로 @나 #를 제거
								len=strlen(tmp2);
								for(i=1;i<len;i++)
									tmp2[i-1]=tmp2[i];
								tmp2[len-1]='\0';

								//ex)@4096, X 같은 경우 예외처리
								if(!test_tail(tmp,start)){	
									err_line=cur_line;error=tail;break;
								}
							}
							else									//simple
								nixbpe[0]=nixbpe[1]=1;
						//1. 만약 뒤에 상수가 온다면 ex)#4096 --> p또는 b비트 설정할 필요 X
							flag=True;len=strlen(tmp2);
							for(i=0;i<len;i++){
								if(!(tmp2[i]>='0' && tmp2[i]<='9')) 
									flag=False;
								}

							//상수가 맞지만 뒤에 뭔가 더 있을때
							if(!test_tail(tmp,start) && flag){ 
								if(get_string(tmp,tmp3,&start,1)==1){
									err_line=cur_line; error=get;break;
								}
								else if(!test_tail(tmp,start)){	
									err_line=cur_line; error=tail;break;
								}
								else if(!strcmp(tmp3,"X\0"))
									nixbpe[2]=1;
							}
							if(flag){ //만약 뒤에 오는 수가 상수라면 

								hexa=string_to_number2(tmp2);
								//만약 4096이상의 수가 왔다면 에러
								if((hexa>=4096 && type==OP3) || (hexa>=0x100000 && type ==OP_PLUS)){	//format3초과 범위의 수일때
									err_line=cur_line; 
									printf("[ERROR]Number boundary error ");
									break;
								}
								
								decInt_to_hexStr(hexa,tmp3,1); //16진수 문자열로 변경후
								for(i=2;i<10;i++) 
									objcode[i]='0';//objcode에 이어붙이기
									
								if(type==OP3){			//format3 일때	
								if(hexa<Pow16[1] && hexa>=0)
									memcpy(objcode+5,tmp3,1);
								else if(hexa<Pow16[2] && hexa>=Pow16[1])
									memcpy(objcode+4,tmp3,2);
								else if(hexa<Pow16[3] && hexa>=Pow16[2])
									memcpy(objcode+3,tmp3,3);
								objcode[6]='\0';
								}
								else{					//format4 일때
									 if(hexa<Pow16[1] && hexa>=0)
										memcpy(objcode+7,tmp3,1);
									else if(hexa<Pow16[2] && hexa>=Pow16[1])
										memcpy(objcode+6,tmp3,2);
									else if(hexa<Pow16[3] && hexa>=Pow16[2])
										memcpy(objcode+5,tmp3,3);
									else if(hexa<Pow16[4] && hexa>=Pow16[3])
										memcpy(objcode+4,tmp3,4);
									else if(hexa<Pow16[5] && hexa>=Pow16[4])
										memcpy(objcode+3,tmp3,5);
								objcode[8]='\0';
								}
							}
							//2. symbol이 온다면 
							else{ //추가해주기 //뒤에오는게 상수가 아닐때
							    /* indexed bit을 설정하는 과정 */
								if((i=find_symbol(tmp2))==-1){ 		//존재하지 않는 sym
										err_line=cur_line;
										printf("[ERROR]No such a symbol exists : %s ",tmp2);
										break; //no existing sym
								}
								else if(!test_tail(tmp,start)){		//symbol table에 존재하지만 뒤에 뭐가 있을때
									if(get_string(tmp,tmp2,&start,1)==1){
										err_line=cur_line; error=get;break;	
									}
									else if(!test_tail(tmp,start)){		
										err_line=cur_line; error=tail;break;
									}
									else if(strcmp(tmp2,"X\0")!=0){		
										err_line=cur_line; 
										printf("[ERROR]Invalid indexed mode ");
										break; //ex)BUFFER, Y
									}
									else
										nixbpe[2]=1;				//x 비트 설정
								}
								/* p와 b bit을 설정하는 과정, i에 symbol값 저장 */
							if(type==OP3){							//format3
								pc=locctr+3;
								if((disp=i-pc)>=-2048 && disp<2048)
									nixbpe[4]=1;
								else if(b && (disp=i-baseLoc)>=0 && disp<4096)
									nixbpe[3]=1;
								else{								//p도 b도 아닐때 에러
									err_line=cur_line;
									printf("[ERROR]Neither PC relative nor BASE relative ");
									break;
								}

								if(disp<0) 							//disp<0이하의 수일때
									decInt_to_hexStr(disp*-1,tmp3,-1);
								else
									decInt_to_hexStr(disp,tmp3,1);
							
								/* objcode의 3,4,5 자리에 문자열 복사하기 */
								if(disp<0){
									disp *= -1;
									for(i=3;i<6;i++) objcode[i]='F';
								}
								else{
									for(i=2;i<6;i++) objcode[i]='0';//objcode에 이어붙이기
								}
								if(disp<16) 
									memcpy(objcode+5,tmp3,1);
								else if(disp<256) 
									memcpy(objcode+4,tmp3,2);
								else if(disp<4096) 
									memcpy(objcode+3,tmp3,3);
								objcode[6]='\0';
							}
							else{									//format4
								/*modification record를 위한 노드 생성*/
								newNode=(modeNode*)malloc(sizeof(modeNode));
								newNode->next=NULL;
								newNode->addr=locctr+1;
								
								if(modi.head==NULL)
									modi.head=newNode;
								else{
									tmpNode=modi.head;
									while(tmpNode->next!=NULL)
										tmpNode=tmpNode->next;
									tmpNode->next=newNode;
								}
								modi.num++;
								/* objcode 저장하기*/
								decInt_to_hexStr(i,tmp3,1);
								for(j=2;j<10;j++)
									objcode[j]='0';					//objcode에 이어붙이기

								if(i<Pow16[1]) 
									memcpy(objcode+7,tmp3,1);
								else if(i<Pow16[2]) 
									memcpy(objcode+6,tmp3,2);
								else if(i<Pow16[3]) 
									memcpy(objcode+5,tmp3,3);
								else if(i<Pow16[4]) 
									memcpy(objcode+4,tmp3,4);
								else if(i<Pow16[5])
									memcpy(objcode+3,tmp3,5);
								objcode[8]='\0';
							}
						}
							/* n,i,x,b,p,e, 비트에 따라 objcode 수정해주기 */
							//1. n,i 계산하기
							c=objcode[1];
							if(c>='0' && c<='9') ni=c-'0';
							else ni=c-'A'+10;

							if(nixbpe[0] && nixbpe[1]) 		 //simple 
								ni += 3;
							else if(nixbpe[0] && !nixbpe[1]) //@indirect 
								ni += 2;
							else if(!nixbpe[0] && nixbpe[1]) //#immediate
								ni += 1;
							
							if(ni>=10)
								objcode[1]='A'+ni-10;
							else
								objcode[1]='0'+ni;
								
							//2. x b p e 계산하기
								xbpe=0;
								if(nixbpe[2])xbpe += 8;
								if(nixbpe[3])xbpe += 4;
								if(nixbpe[4])xbpe += 2;
								if(nixbpe[5])xbpe += 1;

								if(xbpe>=10)
									objcode[2]='A'+xbpe-10;
								else
									objcode[2]='0'+xbpe;
						}	
						fprintf(fp_lst,"%s\n",objcode);
						break;
			case COMMENT : //할거 없음 
						break;
			case BASE : b=True;i=0;
						while((c=fgetc(fp_pass1))!='\n')			//뒤에 있는 문자열 추출(tmp)
							tmp[i++]=c;
						tmp[i]='\0';start=0;
						if(get_string(tmp,tmp2,&start,0)==1){
							err_line=cur_line;error=get; break;		//에러발생
						}
						else if(!test_tail(tmp,start)){
							err_line=cur_line;error=tail; break;	//에러 발생
						}
						else if((i=find_symbol(tmp2))==-1){			//BASE추출
							err_line=cur_line; 
							printf("[ERROR]No such a symbol : %s ",tmp2);
							break;
						}
						else
							baseLoc=i;								//BASE 주소를 저장(int)
						break;
			case NOBASE : b=False;
						  break;
			default : printf("[ERROR]Neither a directive nor a mnemonic "); break;
		}

		/* ERROR code 정리 */
		if(err_line!=-1){
			if(error==get)
				printf("[ERROR]Invalid string ");
			else if(error==tail)
				printf("[ERROR]Too many parameters ");
				
			printf(" (line %d)\n",cur_line*5);
	    	fclose(fp_pass1);fclose(fp_origin);fclose(fp_lst);fclose(fp_obj);
			remove(file1);remove(file2);remove("pass1.txt");
			return -1;
		}

		//2. .obj 파일에 기록하기 위해 정보를 수집
		//.obj에 기록해야한다면 
		if((valid>=30 ||byte+format>30 || type==RESW || type==RESB || type==END)){
			objflag=True;
			if(valid*byte!=0){			//objrec에 아무정보도 없다면 기록X
				fprintf(fp_obj,"T");	//RESW 나 RESB때 이러한 경우 발생
				loc_write_to_file(fp_obj,objLoc,6);
				loc_write_to_file(fp_obj,byte,2);
				for(i=0;i<valid;i++)
					fprintf(fp_obj,"%s",objrecord[i]);
				fprintf(fp_obj,"\n");
				valid=0;byte=0;
				if(type!=RESW && type!=RESB && type!=END && objcode[0]!='\0'){
					strcpy(objrecord[valid++],objcode); 
					byte+=format;
				}
			}
			if(type==END){			//E&M record 기록하고 종료
				if(modi.num!=0){	//메모리 free와 동시에 기록
					while(modi.head){
						tmpNode=modi.head;
						modi.head=modi.head->next;
						fprintf(fp_obj,"M");
						loc_write_to_file(fp_obj,tmpNode->addr,6);
						fprintf(fp_obj,"05\n");
						free(tmpNode);
					}
				}
				fprintf(fp_obj,"E");
				loc_write_to_file(fp_obj,prog_start,6);
				fprintf(fp_obj,"\n");
			}
		 }
		else if(type!=BASE && type!=NOBASE && type!=COMMENT){	
			//obj파일 작성할필요 없다면 정보담아놓기(BASE,NOBASE,COMMENT제외)
			strcpy(objrecord[valid++],objcode);
			byte += format;
		}

		cur_line++;
	if(type==END) break;
	}
	printf("output file : [%s], [%s]\n",file1,file2);

	fclose(fp_origin);fclose(fp_pass1);fclose(fp_lst);fclose(fp_obj);
	return 1;
}
/* 입력 받은 locctr을 파일에 flag자리수로 적어주는 함수 
올바른 locctr(범위 내의 수)만 들어왔다고 가정 */
void loc_write_to_file(FILE* fp,int locctr,int flag){
	if(flag==2){
		if(locctr>=0 && locctr<Pow16[1])
			fprintf(fp,"0%X",locctr);
		else if(locctr<Pow16[2])
			fprintf(fp,"%X",locctr);
	}
	else if(flag==4){
		if(locctr>=0 && locctr<Pow16[1])
			fprintf(fp,"000%X",locctr);
		else if(locctr<Pow16[2])
			fprintf(fp,"00%X",locctr);
		else if(locctr<Pow16[3])
			fprintf(fp,"0%X",locctr);
		else if(locctr<Pow16[4])
			fprintf(fp,"%X",locctr);
	}
	else if(flag==6){
		if(locctr>=0 && locctr<Pow16[1]) 
			fprintf(fp,"00000%X",locctr);
		else if(locctr<Pow16[2]) 
			fprintf(fp,"0000%X",locctr);
		else if(locctr<Pow16[3]) 
			fprintf(fp,"000%X",locctr);
		else if(locctr<Pow16[4]) 
			fprintf(fp,"00%X",locctr);
		else if(locctr<Pow16[5]) 
			fprintf(fp,"0%X",locctr);
		else if(locctr<Pow16[5]*16) 
			fprintf(fp,"%X",locctr);
	}
}
/* 받은 reg를 검사하고 objcode에 덧붙여줌
올바른 reg라면 1반환, 아니라면 -1반환 */
int OP2_choose_reg(char* reg,char *objcode){
	Register r;	//enum형 변수 정의
	char tmp[2];

	if(!strcmp(reg,"A\0")) r=A;
	else if(!strcmp(reg,"X\0")) r=X;
	else if(!strcmp(reg,"L\0")) r=L;
    else if(!strcmp(reg,"B\0")) r=B;
	else if(!strcmp(reg,"S\0")) r=S;
	else if(!strcmp(reg,"T\0")) r=T;
	else if(!strcmp(reg,"F\0")) r=F;
	else if(!strcmp(reg,"PC\0")) r=PC;
	else if(!strcmp(reg,"SW\0")) r=SW;
	else return -1;				//올바르지 않은 reg

	tmp[0]=r+'0';tmp[1]='\0';	//reg붙여넣
	strcat(objcode,tmp);
	
	return 1;
}
/* 10진수 정수를 받아 16진수 문자열로 반환하는 함수 */
void decInt_to_hexStr(int a,char *str,int flag){
	int len=dec_to_hexa_digit(a);
	int i,tmp,tmp2;
	char c;

	str[len]='\0';
	for(i=len-1;i>=0;i--){
		tmp=a%16;
		a /= 16;

		if(tmp<10)
			str[i]=tmp+'0';
		else{
			if(tmp==10)	
				str[i]='A';
			else if(tmp==11) 
				str[i]='B';
			else if(tmp==12) 
				str[i]='C';
			else if(tmp==13) 
				str[i]='D';
			else if(tmp==14) 
				str[i]='E';
			else 
				str[i]='F';
		}
	}
	
	if(flag==-1){		//음수 hexa로 변경
		//1's complement 구하기
		for(i=0;i<len;i++){
			if(str[i]>='0' && str[i]<='9') tmp=str[i]-'0';
			else if(str[i]>='A' && str[i]<='F') tmp=str[i]-'A'+10;

			tmp2=15-tmp;
			
			if(tmp2<10)
				str[i]=tmp2+'0';
			else{
				if(tmp2==10)	
					str[i]='A';
				else if(tmp2==11) 
					str[i]='B';
				else if(tmp2==12) 
					str[i]='C';
				else if(tmp2==13) 
					str[i]='D';
				else if(tmp2==14) 
					str[i]='E';
				else 
					str[i]='F';
			}	
		}
		// 마지막에 +1해주기
		c=str[len-1];
		if((c>='0' && c<='8') || (c>='A' && c<='E')) 
			str[len-1] += 1;
		else if(c=='9')
			str[len-1]='A';
		else if(c=='F'){
			str[len-1]='0';
			c=str[len-2];
			if((c>='0' && c<='8') || (c>='A' && c<='E')) 
				str[len-2] += 1;
			else if(c=='9')
				str[len-2]='A';
			else if(c=='F'){
				str[len-2]='0';
				c=str[len-3];
				if((c>='0' && c<='8') || (c>='A' && c<='E')) 
					str[len-3] += 1;
				else if(c=='9')
					str[len-3]='A';
			}

		}
	}

}
/* symbol table에서 symbol을 찾아서 locctr을 반환하는 함수 */
int find_symbol(char *symbol){
	symbolNode *tmp;

	tmp=symbolTable[symbol[0]-'A'];

	while(tmp!=NULL){
		if(!strcmp(tmp->symbol,symbol)){
			return tmp->line_addr;
		}
		tmp=tmp->next;
	}
	return -1; 	//못찼았다면 -1을 반환 
}
