#include "buffer.h"

int main( int argc, char ** argv ) {

    int fid;
    int input;
    int result;
    char* result2;
    char instruction;
    char buff[120];
    //printf("> ");
    init_db(20);
    fid = open_table("test.db");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
            case 'c':
                //scanf("%d", &input);
                //if(input == fid) fid = 0;
                result = close_table(fid);
                if(result) printf("close table failed\n");
                break;
            case 'd':
                scanf("%d", &input);
                result = delete(fid, input);
                if(result) printf("delete failed\n");
                break;
            case 'o':
                scanf("%s", buff);
                fid = open_table(buff);
                //if(fid != -1) printf("Table ID is %d\n", fid);
                //else printf("Initialize First\n");
                break;
            case 'i':
                scanf("%d %s", &input, buff);
                result = insert(fid, input, buff);
                if(result) printf("insert failed\n");
                break;
            case 'f':
            case 'p':
                scanf("%d", &input);
                result2 = find(fid, input);
                if(result2 != NULL) {
                    strcpy(buff, result2);
                    printf("Key:%d,Value:%s\n",input, buff);
                }
                else printf("Not Exists\n");
                fflush(stdout);
                break;
            case 'r':
                /*scanf("%d %d", &input, &range2);
                  if (input > range2) {
                  int tmp = range2;
                  range2 = input;
                  input = tmp;
                  }
                  find_and_print_range(root, input, range2, instruction == 'p');*/
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
            case 't':
                //print_tree(root);
                break;
            case 'v':
                //verbose_output = !verbose_output;
                break;
            case 'x':
                //if (root)
                //    root = destroy_tree(root);
                //print_tree(root);
                break;
            default:
                //usage_2();
                break;
        }
        while (getchar() != (int)'\n');
        //  printf("> ");
    }
    printf("\n");

    return EXIT_SUCCESS;
}
