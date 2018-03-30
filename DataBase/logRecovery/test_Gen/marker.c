#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "bptree.h"

#define BUF_NUM 3
#define MAX_KEY 300000
#define VALUE_SIZE 120
#define TABLE1_PREFIX "table1"
#define TABLE2_PREFIX "table2"
#define TABLE1_PATH "table1.db"
#define TABLE2_PATH "table2.db"

char bytemap[MAX_KEY];
int64_t key1[MAX_KEY];
int64_t key2[MAX_KEY];

void init_rand(){
	srand((unsigned int)time(NULL));
}

void make_val(const int64_t k, char *v, char *prefix){
	memset(v, 0, VALUE_SIZE);
	sprintf(v, "%s_%ld", prefix, k);
}

//Fisher–Yates shuffle
void make_rand_array(int64_t arr[], int num){
	int i;
	int64_t *dst;
	int64_t temp;
	for (i = 0; i < num; i++){
		arr[i] = i;
	}
	for (i = num-1; i >= 0; i--){
		dst = &arr[rand()%(i+1)];
		temp = arr[i];
		arr[i] = *dst;
		*dst = temp;
	}
}

void panic(const char *msg){
	fprintf(stderr, "%s\n", msg);
	exit(1);
}

void generate_table(){
	char buf[VALUE_SIZE];
	make_rand_array(key1, MAX_KEY);
	make_rand_array(key2, MAX_KEY);
	int table_id1;
	int table_id2;
	int i;

	init_db(BUF_NUM);
	table_id1 = open_table(TABLE1_PATH);
	table_id2 = open_table(TABLE2_PATH);
	for (i = 0; i < MAX_KEY/2 ; i++){
		if (i % 10000 == 0){
			printf("%d\n", i);
		}
		make_val(key1[i], buf, TABLE1_PREFIX);
		if (insert(table_id1, key1[i], buf) != 0){
			panic("insert1");
		}
		make_val(key2[i], buf, TABLE2_PREFIX);
		if (insert(table_id2, key2[i], buf) != 0){
			panic("insert2");
		}
		bytemap[key1[i]]++;
		bytemap[key2[i]]++;
	}
	close_table(table_id1);
	close_table(table_id2);
	shutdown_db();
}

void generate_result(){
	int i;
	char buf1[VALUE_SIZE];
	char buf2[VALUE_SIZE];
	FILE *out = fopen("join_answer.txt","w");
	for (i = 0; i < MAX_KEY; i++){
		if (bytemap[i] == 2){
			make_val(i, buf1, TABLE1_PREFIX);
			make_val(i, buf2, TABLE2_PREFIX);
			fprintf(out, "%d,%s,%d,%s\n", i, buf1, i, buf2);
		}
	}
	fclose(out);
}

int main(int argc, char *argv[]){
	init_rand();
	remove(TABLE1_PATH);
	remove(TABLE2_PATH);
	generate_table();
	generate_result();
	return 0;
}
