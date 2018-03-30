#include <stdint.h>

int init_db(uint64_t buf_size);
int shutdown_db();
int open_table(char *pathname);
int close_table(int table_id);
int insert(int table_id, int64_t key, char *value);
char *find(int table_id, int64_t key);
int delete(int table_id, int64_t key);
//int join_table(int table_id1, int table_id2, char *pathname);
