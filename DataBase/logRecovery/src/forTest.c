#include "recovery.h"

int main( int argc, char ** argv ) {

    int fid;
    int input, input2;
    int result;
    char * result2;
    char instruction;
    char buff[120];
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
            case 'b':
                begin_transaction();
                break;
            case 'm':
                result = commit_transaction();
                if(!result)printf("commit done\n");
                break;
            case 'a':
                result = abort_transaction();
                if(!result)printf("abort done\n");
                break;
            case 'c':
                scanf("%d", &input);
                result = close_table(input);
                if(result) printf("close table failed\n");
                break;
            case 'd':
                scanf("%d %d", &input, &input2);
                result = delete(input, input2);
                if(result) printf("delete failed\n");
                break;
            case 'o':
                scanf("%s", buff);
                open_table(buff);
                if(fid != -1) printf("Table ID is %d\n", fid);
                else printf("Initialize First\n");
                break;
            case 'i':
                scanf("%d %d %s", &input, &input2, buff);
                result = insert(input, input2, buff);
                if(result) printf("insert failed\n");
                break;
            case 'j':
                scanf("%d %d %s", &input, &input2, buff);
                join_table(input, input2, buff);
                break;
            case 'f':
            case 'p':
                scanf("%d %d", &input, &input2);
                result2 = find(input, input2);
                if(result2 != NULL) {
                    strcpy(buff, result2);
                    printf("Key: %d, Value: %s\n",input2, buff);
                }
                else printf("Not Exists\n");
                fflush(stdout);
                break;
            case 's':
                scanf("%d", &input);
                init_db(input);
                break;
            case 'q':
                while (getchar() != (int)'\n');
                shutdown_db();
                return EXIT_SUCCESS;
                break;
            case 'u':
                scanf("%d %d %s", &input, &input2, buff);
                result = update(input, input2, buff);
                if(result) printf("update failed\n");
                break;
            default:
                break;
        }
        while (getchar() != (int)'\n');
    }
    printf("\n");

    return EXIT_SUCCESS;
}
