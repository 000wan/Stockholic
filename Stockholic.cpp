// 
// -------- StockHolic.cpp --------
// --------  2302 �ǿ���  --------
//
// About Program: 
// 	- �ֽĽ��忡�� �������ڿ� �л������� ��� ���ͷ�/����ũ �м� �� �� 
//
// Feature:  
// 	- 0/1 �賶���� (Knapsack) �˰����� �ֽ� ���忡 �ż�/�ŵ� �������� Ȱ�� 
//
// Reference:
//	- ���� ��Ʈ������ �̷� ( https://terms.naver.com/entry.naver?docId=3570360&cid=58783&categoryId=58783 )
//
// Data Source: 
//	- Yahoo Finance ( https://finance.yahoo.com/ )
// 
// History :
// 	2021/3/19(��) ~ 3/25(��) : ���� �˰��� Ȱ���� DP�ۼ� 
// 	2021/5/30(��) ~ 6/1(ȭ)  : ������ ���� �Լ� �ۼ� 
// 	2021/6/4(��) ~ 6/5(��)	 : csv ������ �Է�
// 	2021/6/6(��)			 : �ֽ� ����ü�� �̿��� �ڵ� ����ȭ 
// 	2021/6/7(��) ~ 6/11(��)  : �Է� ���� ����, �Է� �Լ�ȭ �� ����ó�� 
// 							   ��Ʈ������ ���� �� ����ü ���� 
//							   �������ڿ� �л����� �� ���̽� �м����� Ȯ�� 
//	2021/6/12(��) ~ 6/13(��) : �ִ� ���Ͱ� �ִ� �ս� ��� 
//							   �� ��Ʈ�������� ���� �����ͷ� �� ǥ������ ���
//							   ��Ʈ������ �� �м� ��� �߰� 
// 

#include <stdio.h>
#include <stdlib.h> //exit, abs
#include <string.h> //strcmp, strtok
#include <dirent.h> //dir
#include <math.h> //sqrt

#define MAX_DAYS 100 //100�ϰ� �ֽ� �����͸� ������ 
#define MAX_MONEY 5000 //�ִ� �ں����� ���� 
#define MAX_HOLD 100
#define MAX_NAME 20
#define MAX_ITEM 10 //�ֽ� ���� �ִ� ����

const char* market_name[3] = {NULL, "KOSPI", "NASDAQ"}; // 1:KOSPI, 2:NASDAQ
const char* line = "\n\n������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������\n\n"; 
const char sign[2] = {'>', '<'};

typedef struct {
	int fin; //���� �ڱ� 
	char name[MAX_NAME]; //���� �̸� 
	int prices[MAX_DAYS]; //�ְ� ��� 
	
	int memo[MAX_DAYS][MAX_MONEY][MAX_HOLD]; //a�Ϻ��� �ֽ� c�ָ� ������ä�� b�޷��� ������ ����� �ִ� �ִ� ����/�սǱ� 
		//�޸������̼����� �ð����⵵�� ���̱� ���� 
	int record[MAX_DAYS]; //Ž�� ����� ���, �ż�(+) �ŵ�(-)
} Stock; //�ֽ� ����ü 

typedef struct {
	Stock* item; //��Ʈ������ �� Ư�� ������ ������ Stock ������
	int size; //��Ʈ������ ũ�� (�ֽ� ���� ����)
	char list[MAX_ITEM][MAX_NAME]; //�޸� ������ ���� ������� �̸��� ����
	 
	float prof_rate[MAX_ITEM][2]; //[0]:�ִ� ���ͷ� [1]:�ִ� �սǷ�  
	float EXP[MAX_ITEM]; //��� ���ͷ�
	float SD[MAX_ITEM]; //���� ǥ������ 
	float total_EXP, total_SD; //��Ʈ������ ��ü ��� ���ͷ� �� ǥ������ 
	
} Portfolio; //��Ʈ������ ����ü 

void analyze_pf(Portfolio* pf);
int maxProfit(Stock* s, int start, int mon, int hold, int mode);
void solution(Stock* s, int start, int mon, int hold);

void choose_data(Portfolio* Con_Pf, Portfolio* Div_Pf);
void read_data(Stock* s);
void print_result(Portfolio* pf);
void record_cache(Portfolio* pf, int mode);
void compare(Portfolio* Con_Pf, Portfolio* Div_Pf);

int input_select(int n);
void dircat(char* result, char** loc);
void raise_error(char* ErrCode);
void exit_hold();

int market; //�ֽĽ��� 1:KOSPI 2:NASDAQ  
int finance; //�ʱ� �ں� 
int date[MAX_DAYS][3]; // �ֽ��� ���� ��¥ ���

int main() {
	
	printf(" ���������������������������������������������������������������� \n"); 
	printf(" �� [ Stockholic.cpp ] by �ǿ��� �� \n");
	printf(" ���������������������������������������������������������������� \n");
	
	printf("Tip: ���� �߿� ���α׷��� �����ϰ� �����ø� -1�� �Է��ϼ���\n\n");
	
	//Con_Pf : �������� ��Ʈ������ Concentrated Portfolio
	Portfolio* Con_Pf = new Portfolio;  //new�� ���� �޸� �Ҵ�
	//Div_Pf : �л����� ��Ʈ������ Diversified Portfolio
	Portfolio* Div_Pf = new Portfolio;
	
	//������ ����
	choose_data(Con_Pf, Div_Pf);  
	
	//�ʱ� �ں��� �Է�
	printf("\n[III] �ʱ� �ں����� �Է��ϼ��� ($1 ~ $%d) : $ ", (int)(MAX_MONEY*0.4)); //�ִ���ȭ�� 0.4����� �ں��� ���� 
	finance = input_select((int)(MAX_MONEY*0.4));
	
	printf("\n\n������������������������������  �м��� �����մϴ�... ������������������������������ \n\n");
	analyze_pf(Con_Pf);
	printf("\n[I] ��Ʈ������ �м� �Ϸ� \n\n");
	analyze_pf(Div_Pf);
	printf("\n[II] ��Ʈ������ �м� �Ϸ� \n\n");
	printf("\n\n��������������������������������������������������������������������������������  RESULT ���������������������������������������������������������������������������������� \n\n");
	
	printf("Tip: /record ������ ���ø� �ִ� ���Ͱ� �ִ� �ս��� ��� ��츦 ����� txt ������ �ֽ��ϴ�. \n\n");
	
	while(true) {
		printf(" 1: [I] �м���� / 2: [II] �м���� / 3: [I],[II] �� / -1: ���α׷� ���� \n");
		printf("   => ");
		int select = input_select(3);
		printf(line);
		
		switch(select) {
			case 1:
				printf("\n\n\n========== [I] �������� ��Ʈ������ �м���� ========== \n\n");
				print_result(Con_Pf);
				break;
			case 2:
				printf("\n\n\n========== [II] �л����� ��Ʈ������ �м���� ========== \n\n");
				print_result(Div_Pf);
				break;
			case 3:
				compare(Con_Pf, Div_Pf);
				break;
			default: break;
		}
		
		printf(line);
	}
	
	delete Con_Pf; //�޸� free 
	delete Div_Pf;
	exit_hold();
	return 0;
}
 
//������ ���� �Լ� 
void choose_data(Portfolio* Con_Pf, Portfolio* Div_Pf) {
	printf("\n============ < �ֽ� ���� > ============ \n");
	printf("  1. KOSPI \t\t2. NASDAQ\n");
	printf("\n[-] �ֽ� ������ �����ϼ��� (1~2): ");
	
	market = input_select(2); //������ �ֽĽ��� ���� 
	char* locat[3] = { "data", (char*)market_name[market] }; //market 1:KOSPI 2:NASDAQ
	char market_dir[MAX_NAME];
	dircat(market_dir, locat);
	
	DIR* dr = opendir(market_dir); //in data folder
	struct dirent* entry;
	char file_list[MAX_ITEM][MAX_NAME]; 
	
	if(dr == NULL) raise_error("Data Reading Failed");
	else {
		int c = 0; //���� ���� c
		
		printf("\n============ < �ֽ� ���� > ============ \n");
		//data ���� Ž�� 
		for(entry = readdir(dr); entry != NULL; entry = readdir(dr)) {
			char fname[MAX_NAME]; //temporary file name
			strcpy(fname, entry->d_name);
			
			//only csv files
			strtok(fname, ".");
			char* ftype = strtok(NULL, ".");
			
			if(ftype && strcmp(ftype, "csv") == 0) {
				printf("  %d.   %s\n", c+1, fname);
				strcpy(file_list[c], fname);
				c++;
			}
		}
		if(c == 0) raise_error("Data Not Found");
		
		//�������� ������ 
		printf("\n[I] ���������� ������ �����ϼ��� (1~%d): ", c);
		
		int inp = input_select(c);
		Con_Pf->size = 1;
		strcpy(Con_Pf->list[0], file_list[inp-1]);
		printf("\t[1] %s Selected\n", Con_Pf->list[0]);
		
		//�л����� ��Ʈ������ �ۼ�
		printf("\n[II] ��Ʈ�������� ���� ������ �����ϼ��� (1~%d): ", c);
		Div_Pf->size = input_select(c);
		printf("\n");
		
		for(int i = 0; i < Div_Pf->size; i++) {
		 	printf("\t[%d/%d] ������ �����ϼ��� (1~%d): ", i+1, Div_Pf->size, c);
		 	
		 	inp = input_select(c);
			strcpy(Div_Pf->list[i], file_list[inp-1]);
			printf("\t\t[%d/%d] %s Selected\n", i+1, Div_Pf->size, Div_Pf->list[i]);
		}
	}
	
	closedir(dr);
	return;
}

//��Ʈ������ �м� �Լ�
void analyze_pf(Portfolio* pf) {
	pf->total_EXP = pf->total_SD = 0; 
	
	for(int i = 0; i < pf->size; i++) {
		
		pf->item = new Stock;
		//�� �ֽĸ��� Stock�� �����ߴ��� �޸� ���� ���� �߻�.. 
		//=> �޸� ������ ���� pf.item �����͸� �����ذ��� �м�
		
		strcpy(pf->item->name, pf->list[i]);
		read_data(pf->item);
		pf->item->fin = finance / pf->size; // �� ���� ��ü �ں����� 1/n �� ����
		
		memset(pf->item->memo, -1, sizeof(pf->item->memo)); //memo �ʱ�ȭ 
		int mP = maxProfit(pf->item, 0, pf->item->fin, 0, 0); //�ִ� ���� ���  
		solution(pf->item, 0, pf->item->fin, 0);
		float prof_rate = pf->prof_rate[i][0] = pf->item->fin ? mP/(float)pf->item->fin : 0; //�ִ� ���ͷ� ����
		record_cache(pf, 0);
		
		memset(pf->item->memo, -1, sizeof(pf->item->memo));
		int mL = maxProfit(pf->item, 0, pf->item->fin, 0, 1); //�ִ� �ս� ��� 
		solution(pf->item, 0, pf->item->fin, 0);
		float loss_rate = pf->prof_rate[i][1] = pf->item->fin ? mL/(float)pf->item->fin : 0;
		record_cache(pf, 1);
		
		if(prof_rate == 0.f && loss_rate == 0.f)
			pf->EXP[i] = pf->SD[i] = 0;
		else {
			float p1 = fabs(prof_rate)/(fabs(prof_rate)+fabs(loss_rate)); //���� ���� Ȯ�� ��� p1:�ִ� ���� Ȯ��, p2:�ִ� �ս� Ȯ�� 
			float p2 = fabs(loss_rate)/(fabs(prof_rate)+fabs(loss_rate)); 
			pf->EXP[i] = prof_rate * p1  + loss_rate * p2; //�ִ� ���ͷ�� �սǷ��� �������(���) ��� 
			pf->SD[i] = sqrt(pow(prof_rate - pf->EXP[i], 2) * p1 + pow(loss_rate - pf->EXP[i], 2) * p2); //���ͷ� ǥ������ ��� 
		}
		
		pf->total_EXP += pf->EXP[i];
		pf->total_SD += pf->SD[i];
	}
	
	//��Ʈ�������� ��� ���ͷ� �� ǥ������ ��� 
	pf->total_EXP /= pf->size;
	pf->total_SD /= pf->size;
}

//�ִ� ���ͱ� �м� �Լ� (mode = 0 �̸� �ִ� ����, mode = 1�̸� �ִ� �ս� ���) 
int maxProfit(Stock* s, int start, int mon, int hold, int mode) {
	//memoization
	if(s->memo[start][mon][hold] == -1)
	{
		if(start == MAX_DAYS-1) {
			s->memo[start][mon][hold] = hold * s->prices[start];
			return s->memo[start][mon][hold];
		}
		
		int MP = mode == 0 ? -1 : MAX_MONEY; //start�Ϻ��� ���� �� �ִ� �ִ� ����/�սǱ�
		//�ִ� ������ ���Ҷ� �ʱⰪ�� �ּҷ� ��� �ִ� �ս��� ���Ҷ� �ʱⰪ�� �ִ�� ���� 
		int able = mon/s->prices[start]; //start�� �ż��� �� �ִ� �ִ뷮
		
		for(int i = able; i >= -hold; i--) { //start�� �ŵ��� �� �ִ� �ִ뷮 = hold 
		//i�� �ż� (+) or �ŵ� (-)
				int next = -(i*s->prices[start]) + maxProfit(s, start+1, mon-i*s->prices[start], hold+i, mode);
				//i�ָ� �ż�/�ŵ� ������ ��� ���� 
				
				if(mode == 0 && next > MP)
					MP = next; //�ִ� ���� ���� 
				else if(mode == 1 && next < MP)
					MP = next; //�ִ� �ս� ����
					 
				//DFS
		}
		
		s->memo[start][mon][hold] = MP;
	}
	
	return s->memo[start][mon][hold];
}


//�߰��� ������ Ž�� 
void solution(Stock* s, int start, int mon, int hold) {
	if(start == MAX_DAYS-1) {
		s->record[start] = -hold;
		return;
	}
	
	int able = mon/s->prices[start];
		
	for(int i = able; i >= -hold; i--) {
		//i�� buy (+) or sell (-)
			int next = -(i*s->prices[start]) + s->memo[start+1][mon-i*s->prices[start]][hold+i];
			
			if(s->memo[start][mon][hold] == next) { // i�� �ż�/�ŵ� �������� memo�� �������� ��� 
				s->record[start] = i;
				solution(s, start+1, mon-i*s->prices[start], hold+i);
				break;
			}
	}
	
	return;
} 

void print_result(Portfolio* pf) {
	printf("                     \t��\n");
	printf("         ����       \t��\t �ִ���ͷ�\t�ִ�սǷ�\t�����ͷ�\tǥ������ \n");
	printf("                     \t��\n");
	printf("�������������������������������������������������������������������������������������������������������������������������������������������������������������������������������������� \n");
	printf("                     \t��\n");
	for(int i = 0; i < pf->size; i++) {
		char allign_name[MAX_NAME];
		strcpy(allign_name, pf->list[i]);
		for(int c = strlen(allign_name); c<MAX_NAME-1; c++) allign_name[c] = ' ';
		allign_name[MAX_NAME] = '\0';
		printf(" %s\t��\t %.2f %% \t %.2f %% \t %.2f %% \t %.2f\t \n", allign_name, 100*pf->prof_rate[i][0],  100*pf->prof_rate[i][1], 100*pf->EXP[i], pf->SD[i]);
		printf("                     \t��\n");
	}
	
	printf("\n  ��Ʈ������ ��ü �����ͷ� => %.2f %% , ǥ������ => %.2f\n\n", 100*pf->total_EXP, pf->total_SD);
}

void compare(Portfolio* Con_Pf, Portfolio* Div_Pf) {
	printf("\n\n\n========== [I] vs [II] ��Ʈ������ �񱳰�� ========== \n\n\n");
	printf("        \t �������� ��Ʈ������\t �л����� ��Ʈ������ \n");
	printf("���������������������������������������������������������������������������������������������������������������������������������� \n");
	
	int flag1 ,flag2; // bigger one
	flag1 = (Con_Pf->total_EXP > Div_Pf->total_EXP) ? 0 : 1;
	printf(" �����ͷ� :\t\t%.2f %% \t %c \t %.2f %% \n\n", 100*Con_Pf->total_EXP, sign[flag1], 100*Div_Pf->total_EXP);
	
	flag2 = (Con_Pf->total_SD > Div_Pf->total_SD) ? 0 : 1;
	printf("  ǥ�� ���� :\t\t%.2f %% \t %c \t %.2f %% \n\n", Con_Pf->total_SD, sign[flag2], Div_Pf->total_SD);
	
	if(flag1 == 0) {
		if(flag2 == 0)
			printf(" \"���� ����ũ, ���� ����. (High Risk, High Return)\" \n\n"); 
			//�������ڰ� �����ͷ�+ǥ������ ��� �� ū ��� 
		else printf(" \"�л����ڵ� ���� ����(Market Risk), ���� ü���� ����(Systemic Risk)�� ���� ���� ����. \" \n\n"); 
		//�������ڰ� �����ͷ��� ũ�� ǥ�������� �� ���� ��� 
	}
	else printf(" \"�ް��� �� �ٱ��Ͽ� ���� ����. (Don't Put All Your Eggs in One Basket)\" \n\n");
	//�л����ڰ� �����ͷ��� �� ū ��� 
}

void record_cache(Portfolio* pf, int mode) {
	FILE* pFile;
    char* locat[3] = { "record", pf->item->name }; //txt ���� ��ġ 
    char dir[MAX_NAME];
	dircat(dir, locat);
    
    Stock* s = pf->item;
    pFile = fopen(strcat(dir, ".txt"), mode == 0 ? "w" : "a");
	
    if(pFile == NULL) raise_error("File Open Failed");
	else {
		fprintf(pFile, "[%s] Max %s Case\n\n", s->name, mode == 0 ? "Profit" : "Loss");
		
		int profit = s->memo[0][s->fin][0];
		float prate = s->fin ? 100*profit/(float)s->fin : 0;
		fprintf(pFile, "����: $ %d \t ���ͱ�: $ %d \t ���ͷ�: %.2f %% \n\n", s->fin, profit, prate);
		
		int h = 0;
		int m = s->fin;
		for(int i = 0; i < MAX_DAYS; i++) {
			h += s->record[i];
			m -= s->prices[i]*s->record[i];
			fprintf(pFile, "D+%d\t %d/%d/%d\t Price: $ %d\t buy/sell: %d\t hold: %d\t money: $ %d \n", i, date[i][0], date[i][1], date[i][2], s->prices[i], s->record[i], h, m);
		}
		
		fprintf(pFile, "\n==============================\n\n");
	}
	
	fclose(pFile);
	printf("%s Complete [%d/2]\n", s->name, mode+1);
}

//csv read �Լ� 
void read_data(Stock* s) {
	
    FILE* pFile;
    char* locat[4] = { "data", (char*)market_name[market], s->name }; //csv ���� ��ġ 
    char dir[MAX_NAME];
	dircat(dir, locat);
    
    pFile = fopen(strcat(dir, ".csv"), "r");
	
    if(pFile == NULL) raise_error("File Reading Failed");
	else {
		char* title = new char[50];
		fgets(title, 50, pFile); //ù�� �ѱ��
		delete title;
		
		for(int i = 0; i < MAX_DAYS; i++) {
			if(feof(pFile)) raise_error("Lack of Data");
			float open, close;
            fscanf(pFile, "%d-%d-%d,%f,%*f,%*f,%f,%*f,%*d \n", &date[i][0], &date[i][1], &date[i][2], &open, &close);       
            //YYYY-MM-DD,Open,High,Low,Close,Adj Close,Volume \n 
			int price = (open+close)/2; //�ð��� ������ ������� �ش� ���� �ְ� ���� 
			if(market == 1) price /= 1100; //KOSPI ���� �޷��� ȯ��
			s->prices[i] = price;
        }  
	}
    
	fclose(pFile);
	return;
}

//1~n �Է� �Լ� 
int input_select(int n) {
	while(true) {
		int inp;
		fflush(stdin);
		scanf("%d", &inp);
		
		if(inp == -1) {
			printf("\n[���α׷��� �����մϴ�]\n");
			exit_hold();
			return 0;
		}
		else if(inp>n || inp<1)
			printf("## �Է¹��� ���Դϴ�. �ٽ� �������ּ��� (1~%d): ",n);
		else return inp;
	}
}

//���� ��ġ ���ڿ� ���� �Լ� 
void dircat(char* result, char** loc) {
	char res[MAX_NAME] = "";
	while(*loc != NULL) {
		strcat(res, *(loc++));
		strcat(res, "/");
	}
	res[strlen(res)-1] = '\0';
	
	strcpy(result, res);
	return;
}

//���� �߻� �� ���� 
void raise_error(char* ErrCode) {
	printf("\n[An Error Occured!] : ");
	printf("%s\n", ErrCode);
	exit_hold();
}

//���α׷� ���� 
void exit_hold()
{
	fflush(stdin);
	printf("\n========== Enter�� ������ ����˴ϴ�. ==========\n");
	getchar();
	exit(0);
	
	return;
}
