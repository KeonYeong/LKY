#include "diskBasedBTree.h"

int main( int argc, char ** argv ) {

    char * input_file;
    FILE * fp;
    int input, range2;
    char instruction;
    char license_part;
	char buff[120];
    //printf("> ");
	open_db("test.db");
    while (scanf("%c", &instruction) != EOF) {
        switch (instruction) {
        case 'd':
            scanf("%d", &input);
            delete(input);
            break;
		case 'o':
			scanf("%s", buff);
			open_db(buff);
			break;
        case 'i':
            scanf("%d %s", &input, buff);
            insert(input, buff);
            break;
        case 'f':
        case 'p':
            scanf("%d", &input);
			if(find(input) != NULL) printf("Key:%d,Value:%s\n",input, find(input));
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
        case 'l':
            print_file();
            break;
        case 'q':
            while (getchar() != (int)'\n');
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
