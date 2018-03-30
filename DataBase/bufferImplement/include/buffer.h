#ifndef __BPT_H__
#define __BPT_H__
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>

#define LEAF_ORDER 32
#define INTERNAL_ORDER 249
#define TABLENUM 10
#define ulint unsigned long int
#define PGSIZE 4096


/* Disk-based B+ Tree Implementation */
// 노드들은 다 key와 그에 맞는 주소값을 가지고 있는 배열을
// 각 한개씩 두개 들고 있다. 
// 또한 pageOff 에는 부모를, extraOff은 리프일 경우 다음 리프를 internal일 경우 키보다 작은 모든 데이터의 포인터
typedef struct page{
    ulint pageOff;	// 만약 free page 라면 다음 free page를 가리킴
    char isLeaf;	// 0 = internal, 1 = leaf
    unsigned int numKeys;
    int dummy[26];	// dummy는 스키마에 맞추기 위해 집어넣었다.
    ulint extraOff;
    union{
        struct leafRec{
            int64_t key;
            char value[120];
        }lRec[31];
        struct internalRec{
            int64_t key;
            ulint pageOff;
        }iRec[248];
    }rec;
}page;

typedef struct h_page{
	ulint freePageOff;
	ulint rootPageOff;
	ulint numPages;
    int dummy[1018];
}h_page;

// 버퍼 테이블 구조체
typedef struct _Buffer{
    page frame;
    int table_id;
    ulint pageOff;
    int is_dirty;
    int pin_count;
    int LRU;
} Buffer;

// 테이블 맵핑 구조체
typedef struct _Table{
    int fd;
    char name[120];
    char inUse;
    h_page header;
}Table;

// GLOBALS, 설명들은 .c파일에 있다.
extern ulint currentOff;
extern ulint addedOff;
extern ulint remainingOff;
extern page* buffPage;
extern page* neighbor;
extern char retBuff[120];

// FUNCTION PROTOTYPES.
void fileWrite(ulint offset, page* buff, int fd);
void fileRead(ulint offset, page* buff, int fd);
void find_leaf(int table_id, int64_t key);
int cut( int length );

// user api
int open_table(char * pathname);	//path 를 받아서 fd 파일디스크립터에 넣는다. 성공하면 0 반환 아니면 0이 아닌 값 반환
int insert(int table_id, int64_t key, char * value);	//키와 value를 받는다, find 해서 적절한 노드에다가 추가하는데, 만약 넘으면 페이지를 하나 더 할당받고 거기다 추가함 페이지들이 노드임, 성공하면 0을 반환하고 아니면 0이 아닌 값을 반환
int delete(int table_id, int64_t key);	//키값을 받고, 그 키값에 해당되는 데이터를 삭제한다, 만약 조정이 필요하면 조정을 하고 그 과정중 페이지가 필요가 없어진다면 그 페이지를 완전히 비우고 free page list에 추가한다. 성공하면 0 반환 아니면 0이 아닌값 반환
char* find(int table_id, int64_t key);		//키값을 받아서 그 키값에 해당되는 value 를 반환함, 만약 없으면 NULL

// 내부 함수
// Insertion.

void gv_init (void);
int get_left_index(ulint curOff);
void insert_into_leaf(int table_id, int64_t key, char* value);
void insert_into_leaf_after_splitting(int table_id, int64_t key, char* value);
void insert_into_node(int table_id, int left_index, int64_t key);
void insert_into_node_after_splitting(int table_id, int left_index, int64_t key);
void insert_into_parent(int table_id, int64_t key);
void insert_into_new_root(int table_id, ulint leftOff, int64_t key);
void start_new_tree(int table_id, int64_t key, char* value);

// Deletion.

void adjust_root(int table_id);
void coalesce_nodes(int table_id, int neighbor_index, int64_t k_prime, int n_idx, page* parent, int p_idx);
void redistribute_nodes(int table_id, int neighbor_index,int k_prime_index, int64_t k_prime, page* parent);
void delete_entry(int table_id, int64_t key);

/* Buffer Implementation */

// 버퍼 관련 전역 변수
extern Buffer * buffTable; // 버퍼테이블 참조 포인터
extern int buffNum;
extern Table* tables;
extern int clockHand;
extern int currentInBuff;

// 버퍼 함수들
int init_db(int buf_num);
int close_table(int table_id);

// 버퍼 사용 함수
int bufferWrite(ulint offset, page* buff, int table_id);
int bufferRead(ulint offset, page* buff, int table_id);

// 핀카운트 조정 함수
void appointFrame(int idx);
void terminateFrame(int idx);

// 버퍼 관리 함수
int selectVictim(int table_id);
int shutdown_db(void);
void frame_init(int idx, int table_id, ulint offset);

#endif /* __BPT_H__*/
