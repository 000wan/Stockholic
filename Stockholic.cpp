// 
// -------- StockHolic.cpp --------
// --------  2302 권영완  --------
//
// About Program: 
// 	- 주식시장에서 집중투자와 분산투자의 기대 수익률/리스크 분석 및 비교 
//
// Feature:  
// 	- 0/1 배낭문제 (Knapsack) 알고리즘을 주식 시장에 매수/매도 개념으로 활용 
//
// Reference:
//	- 현대 포트폴리오 이론 ( https://terms.naver.com/entry.naver?docId=3570360&cid=58783&categoryId=58783 )
//
// Data Source: 
//	- Yahoo Finance ( https://finance.yahoo.com/ )
// 
// History :
// 	2021/3/19(금) ~ 3/25(목) : 냅색 알고리즘 활용해 DP작성 
// 	2021/5/30(일) ~ 6/1(화)  : 최적해 저장 함수 작성 
// 	2021/6/4(금) ~ 6/5(토)	 : csv 데이터 입력
// 	2021/6/6(일)			 : 주식 구조체를 이용해 코드 구조화 
// 	2021/6/7(월) ~ 6/11(금)  : 입력 파일 선택, 입력 함수화 및 예외처리 
// 							   포트폴리오 개념 및 구조체 도입 
//							   집중투자와 분산투자 두 케이스 분석으로 확장 
//	2021/6/12(토) ~ 6/13(일) : 최대 이익과 최대 손실 계산 
//							   각 포트폴리오에 대해 기대수익률 및 표준편차 계산
//							   포트폴리오 비교 분석 기능 추가 
// 

#include <stdio.h>
#include <stdlib.h> //exit, abs
#include <string.h> //strcmp, strtok
#include <dirent.h> //dir
#include <math.h> //sqrt

#define MAX_DAYS 100 //100일간 주식 데이터를 수집함 
#define MAX_MONEY 5000 //최대 자본금을 설정 
#define MAX_HOLD 100
#define MAX_NAME 20
#define MAX_ITEM 10 //주식 종목 최대 개수

const char* market_name[3] = {NULL, "KOSPI", "NASDAQ"}; // 1:KOSPI, 2:NASDAQ
const char* line = "\n\n──────────────────────────────────────────────────────────────────────────────────────────\n\n"; 
const char sign[2] = {'>', '<'};

typedef struct {
	int fin; //투자 자금 
	char name[MAX_NAME]; //종목 이름 
	int prices[MAX_DAYS]; //주가 기록 
	
	int memo[MAX_DAYS][MAX_MONEY][MAX_HOLD]; //a일부터 주식 c주를 보유한채로 b달러를 가지고 만들수 있는 최대 이익/손실금 
		//메모이제이션으로 시간복잡도를 줄이기 위함 
	int record[MAX_DAYS]; //탐색 결과를 기록, 매수(+) 매도(-)
} Stock; //주식 구조체 

typedef struct {
	Stock* item; //포트폴리오 내 특정 종목을 지정할 Stock 포인터
	int size; //포트폴리오 크기 (주식 종목 개수)
	char list[MAX_ITEM][MAX_NAME]; //메모리 절약을 위해 종목들의 이름만 저장
	 
	float prof_rate[MAX_ITEM][2]; //[0]:최대 수익률 [1]:최대 손실률  
	float EXP[MAX_ITEM]; //기대 수익률
	float SD[MAX_ITEM]; //수익 표준편차 
	float total_EXP, total_SD; //포트폴리오 전체 기대 수익률 및 표준편차 
	
} Portfolio; //포트폴리오 구조체 

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

int market; //주식시장 1:KOSPI 2:NASDAQ  
int finance; //초기 자본 
int date[MAX_DAYS][3]; // 주식장 열린 날짜 기록

int main() {
	
	printf(" ┌──────────────────────────────┐ \n"); 
	printf(" │ [ Stockholic.cpp ] by 권영완 │ \n");
	printf(" └──────────────────────────────┘ \n");
	
	printf("Tip: 실행 중에 프로그램을 종료하고 싶으시면 -1을 입력하세요\n\n");
	
	//Con_Pf : 집중투자 포트폴리오 Concentrated Portfolio
	Portfolio* Con_Pf = new Portfolio;  //new로 동적 메모리 할당
	//Div_Pf : 분산투자 포트폴리오 Diversified Portfolio
	Portfolio* Div_Pf = new Portfolio;
	
	//데이터 선택
	choose_data(Con_Pf, Div_Pf);  
	
	//초기 자본금 입력
	printf("\n[III] 초기 자본금을 입력하세요 ($1 ~ $%d) : $ ", (int)(MAX_MONEY*0.4)); //최대재화의 0.4배까지 자본금 가능 
	finance = input_select((int)(MAX_MONEY*0.4));
	
	printf("\n\n───────────────  분석을 시작합니다... ─────────────── \n\n");
	analyze_pf(Con_Pf);
	printf("\n[I] 포트폴리오 분석 완료 \n\n");
	analyze_pf(Div_Pf);
	printf("\n[II] 포트폴리오 분석 완료 \n\n");
	printf("\n\n────────────────────────────────────────  RESULT ───────────────────────────────────────── \n\n");
	
	printf("Tip: /record 폴더에 가시면 최대 이익과 최대 손실을 얻는 경우를 기록한 txt 파일이 있습니다. \n\n");
	
	while(true) {
		printf(" 1: [I] 분석결과 / 2: [II] 분석결과 / 3: [I],[II] 비교 / -1: 프로그램 종료 \n");
		printf("   => ");
		int select = input_select(3);
		printf(line);
		
		switch(select) {
			case 1:
				printf("\n\n\n========== [I] 집중투자 포트폴리오 분석결과 ========== \n\n");
				print_result(Con_Pf);
				break;
			case 2:
				printf("\n\n\n========== [II] 분산투자 포트폴리오 분석결과 ========== \n\n");
				print_result(Div_Pf);
				break;
			case 3:
				compare(Con_Pf, Div_Pf);
				break;
			default: break;
		}
		
		printf(line);
	}
	
	delete Con_Pf; //메모리 free 
	delete Div_Pf;
	exit_hold();
	return 0;
}
 
//데이터 선택 함수 
void choose_data(Portfolio* Con_Pf, Portfolio* Div_Pf) {
	printf("\n============ < 주식 시장 > ============ \n");
	printf("  1. KOSPI \t\t2. NASDAQ\n");
	printf("\n[-] 주식 시장을 선택하세요 (1~2): ");
	
	market = input_select(2); //선택한 주식시장 저장 
	char* locat[3] = { "data", (char*)market_name[market] }; //market 1:KOSPI 2:NASDAQ
	char market_dir[MAX_NAME];
	dircat(market_dir, locat);
	
	DIR* dr = opendir(market_dir); //in data folder
	struct dirent* entry;
	char file_list[MAX_ITEM][MAX_NAME]; 
	
	if(dr == NULL) raise_error("Data Reading Failed");
	else {
		int c = 0; //파일 개수 c
		
		printf("\n============ < 주식 종목 > ============ \n");
		//data 폴더 탐색 
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
		
		//집중투자 종목선택 
		printf("\n[I] 집중투자할 종목을 선택하세요 (1~%d): ", c);
		
		int inp = input_select(c);
		Con_Pf->size = 1;
		strcpy(Con_Pf->list[0], file_list[inp-1]);
		printf("\t[1] %s Selected\n", Con_Pf->list[0]);
		
		//분산투자 포트폴리오 작성
		printf("\n[II] 포트폴리오의 종목 개수를 선택하세요 (1~%d): ", c);
		Div_Pf->size = input_select(c);
		printf("\n");
		
		for(int i = 0; i < Div_Pf->size; i++) {
		 	printf("\t[%d/%d] 종목을 선택하세요 (1~%d): ", i+1, Div_Pf->size, c);
		 	
		 	inp = input_select(c);
			strcpy(Div_Pf->list[i], file_list[inp-1]);
			printf("\t\t[%d/%d] %s Selected\n", i+1, Div_Pf->size, Div_Pf->list[i]);
		}
	}
	
	closedir(dr);
	return;
}

//포트폴리오 분석 함수
void analyze_pf(Portfolio* pf) {
	pf->total_EXP = pf->total_SD = 0; 
	
	for(int i = 0; i < pf->size; i++) {
		
		pf->item = new Stock;
		//각 주식마다 Stock을 선언했더니 메모리 부족 문제 발생.. 
		//=> 메모리 절약을 위해 pf.item 포인터를 갱신해가며 분석
		
		strcpy(pf->item->name, pf->list[i]);
		read_data(pf->item);
		pf->item->fin = finance / pf->size; // 한 종목에 전체 자본금의 1/n 씩 투자
		
		memset(pf->item->memo, -1, sizeof(pf->item->memo)); //memo 초기화 
		int mP = maxProfit(pf->item, 0, pf->item->fin, 0, 0); //최대 수익 계산  
		solution(pf->item, 0, pf->item->fin, 0);
		float prof_rate = pf->prof_rate[i][0] = pf->item->fin ? mP/(float)pf->item->fin : 0; //최대 수익률 저장
		record_cache(pf, 0);
		
		memset(pf->item->memo, -1, sizeof(pf->item->memo));
		int mL = maxProfit(pf->item, 0, pf->item->fin, 0, 1); //최대 손실 계산 
		solution(pf->item, 0, pf->item->fin, 0);
		float loss_rate = pf->prof_rate[i][1] = pf->item->fin ? mL/(float)pf->item->fin : 0;
		record_cache(pf, 1);
		
		if(prof_rate == 0.f && loss_rate == 0.f)
			pf->EXP[i] = pf->SD[i] = 0;
		else {
			float p1 = fabs(prof_rate)/(fabs(prof_rate)+fabs(loss_rate)); //손익 실현 확률 계산 p1:최대 이익 확률, p2:최대 손실 확률 
			float p2 = fabs(loss_rate)/(fabs(prof_rate)+fabs(loss_rate)); 
			pf->EXP[i] = prof_rate * p1  + loss_rate * p2; //최대 수익룰과 손실률의 가중평균(기댓값) 계산 
			pf->SD[i] = sqrt(pow(prof_rate - pf->EXP[i], 2) * p1 + pow(loss_rate - pf->EXP[i], 2) * p2); //수익률 표준편차 계산 
		}
		
		pf->total_EXP += pf->EXP[i];
		pf->total_SD += pf->SD[i];
	}
	
	//포트폴리오의 평균 수익률 및 표준편차 계산 
	pf->total_EXP /= pf->size;
	pf->total_SD /= pf->size;
}

//최대 손익금 분석 함수 (mode = 0 이면 최대 이익, mode = 1이면 최대 손실 계산) 
int maxProfit(Stock* s, int start, int mon, int hold, int mode) {
	//memoization
	if(s->memo[start][mon][hold] == -1)
	{
		if(start == MAX_DAYS-1) {
			s->memo[start][mon][hold] = hold * s->prices[start];
			return s->memo[start][mon][hold];
		}
		
		int MP = mode == 0 ? -1 : MAX_MONEY; //start일부터 얻을 수 있는 최대 이익/손실금
		//최대 이익을 구할땐 초기값을 최소로 잡고 최대 손실을 구할땐 초기값을 최대로 잡음 
		int able = mon/s->prices[start]; //start일 매수할 수 있는 최대량
		
		for(int i = able; i >= -hold; i--) { //start일 매도할 수 있는 최대량 = hold 
		//i주 매수 (+) or 매도 (-)
				int next = -(i*s->prices[start]) + maxProfit(s, start+1, mon-i*s->prices[start], hold+i, mode);
				//i주를 매수/매도 했을때 얻는 손익 
				
				if(mode == 0 && next > MP)
					MP = next; //최대 이익 갱신 
				else if(mode == 1 && next < MP)
					MP = next; //최대 손실 갱신
					 
				//DFS
		}
		
		s->memo[start][mon][hold] = MP;
	}
	
	return s->memo[start][mon][hold];
}


//발견한 최적해 탐색 
void solution(Stock* s, int start, int mon, int hold) {
	if(start == MAX_DAYS-1) {
		s->record[start] = -hold;
		return;
	}
	
	int able = mon/s->prices[start];
		
	for(int i = able; i >= -hold; i--) {
		//i개 buy (+) or sell (-)
			int next = -(i*s->prices[start]) + s->memo[start+1][mon-i*s->prices[start]][hold+i];
			
			if(s->memo[start][mon][hold] == next) { // i개 매수/매도 했을때가 memo된 최적해인 경우 
				s->record[start] = i;
				solution(s, start+1, mon-i*s->prices[start], hold+i);
				break;
			}
	}
	
	return;
} 

void print_result(Portfolio* pf) {
	printf("                     \t│\n");
	printf("         종목       \t│\t 최대수익률\t최대손실률\t기대수익률\t표준편차 \n");
	printf("                     \t│\n");
	printf("────────────────────────│────────────────────────────────────────────────────────────────── \n");
	printf("                     \t│\n");
	for(int i = 0; i < pf->size; i++) {
		char allign_name[MAX_NAME];
		strcpy(allign_name, pf->list[i]);
		for(int c = strlen(allign_name); c<MAX_NAME-1; c++) allign_name[c] = ' ';
		allign_name[MAX_NAME] = '\0';
		printf(" %s\t│\t %.2f %% \t %.2f %% \t %.2f %% \t %.2f\t \n", allign_name, 100*pf->prof_rate[i][0],  100*pf->prof_rate[i][1], 100*pf->EXP[i], pf->SD[i]);
		printf("                     \t│\n");
	}
	
	printf("\n  포트폴리오 전체 기대수익률 => %.2f %% , 표준편차 => %.2f\n\n", 100*pf->total_EXP, pf->total_SD);
}

void compare(Portfolio* Con_Pf, Portfolio* Div_Pf) {
	printf("\n\n\n========== [I] vs [II] 포트폴리오 비교결과 ========== \n\n\n");
	printf("        \t 집중투자 포트폴리오\t 분산투자 포트폴리오 \n");
	printf("───────────────────────────────────────────────────────────────── \n");
	
	int flag1 ,flag2; // bigger one
	flag1 = (Con_Pf->total_EXP > Div_Pf->total_EXP) ? 0 : 1;
	printf(" 기대수익률 :\t\t%.2f %% \t %c \t %.2f %% \n\n", 100*Con_Pf->total_EXP, sign[flag1], 100*Div_Pf->total_EXP);
	
	flag2 = (Con_Pf->total_SD > Div_Pf->total_SD) ? 0 : 1;
	printf("  표준 편차 :\t\t%.2f %% \t %c \t %.2f %% \n\n", Con_Pf->total_SD, sign[flag2], Div_Pf->total_SD);
	
	if(flag1 == 0) {
		if(flag2 == 0)
			printf(" \"하이 리스크, 하이 리턴. (High Risk, High Return)\" \n\n"); 
			//집중투자가 기대수익률+표준편차 모두 더 큰 경우 
		else printf(" \"분산투자도 시장 위험(Market Risk), 이하 체계적 위험(Systemic Risk)을 피할 수는 없다. \" \n\n"); 
		//집중투자가 기대수익률은 크나 표준편차는 더 작은 경우 
	}
	else printf(" \"달걀을 한 바구니에 담지 말라. (Don't Put All Your Eggs in One Basket)\" \n\n");
	//분산투자가 기대수익률이 더 큰 경우 
}

void record_cache(Portfolio* pf, int mode) {
	FILE* pFile;
    char* locat[3] = { "record", pf->item->name }; //txt 파일 위치 
    char dir[MAX_NAME];
	dircat(dir, locat);
    
    Stock* s = pf->item;
    pFile = fopen(strcat(dir, ".txt"), mode == 0 ? "w" : "a");
	
    if(pFile == NULL) raise_error("File Open Failed");
	else {
		fprintf(pFile, "[%s] Max %s Case\n\n", s->name, mode == 0 ? "Profit" : "Loss");
		
		int profit = s->memo[0][s->fin][0];
		float prate = s->fin ? 100*profit/(float)s->fin : 0;
		fprintf(pFile, "원금: $ %d \t 수익금: $ %d \t 수익률: %.2f %% \n\n", s->fin, profit, prate);
		
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

//csv read 함수 
void read_data(Stock* s) {
	
    FILE* pFile;
    char* locat[4] = { "data", (char*)market_name[market], s->name }; //csv 파일 위치 
    char dir[MAX_NAME];
	dircat(dir, locat);
    
    pFile = fopen(strcat(dir, ".csv"), "r");
	
    if(pFile == NULL) raise_error("File Reading Failed");
	else {
		char* title = new char[50];
		fgets(title, 50, pFile); //첫줄 넘기기
		delete title;
		
		for(int i = 0; i < MAX_DAYS; i++) {
			if(feof(pFile)) raise_error("Lack of Data");
			float open, close;
            fscanf(pFile, "%d-%d-%d,%f,%*f,%*f,%f,%*f,%*d \n", &date[i][0], &date[i][1], &date[i][2], &open, &close);       
            //YYYY-MM-DD,Open,High,Low,Close,Adj Close,Volume \n 
			int price = (open+close)/2; //시가와 종가의 평균으로 해당 일의 주가 설정 
			if(market == 1) price /= 1100; //KOSPI 종목 달러로 환산
			s->prices[i] = price;
        }  
	}
    
	fclose(pFile);
	return;
}

//1~n 입력 함수 
int input_select(int n) {
	while(true) {
		int inp;
		fflush(stdin);
		scanf("%d", &inp);
		
		if(inp == -1) {
			printf("\n[프로그램을 종료합니다]\n");
			exit_hold();
			return 0;
		}
		else if(inp>n || inp<1)
			printf("## 입력범위 밖입니다. 다시 선택해주세요 (1~%d): ",n);
		else return inp;
	}
}

//파일 위치 문자열 생성 함수 
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

//에러 발생 및 종료 
void raise_error(char* ErrCode) {
	printf("\n[An Error Occured!] : ");
	printf("%s\n", ErrCode);
	exit_hold();
}

//프로그램 종료 
void exit_hold()
{
	fflush(stdin);
	printf("\n========== Enter를 누르면 종료됩니다. ==========\n");
	getchar();
	exit(0);
	
	return;
}
