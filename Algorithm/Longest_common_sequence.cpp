//2013012115_¿Ã∞«øµ
 
#include <stdio.h>
#include <string.h>  
#include <stdlib.h>
 
using namespace std;
 
int main() {
    int LCS_length = 0;
	int max;
    int i, j;
	int **table;
    char a[10000], b[10000], tmp[10000];
    scanf("%s", a+1);
    scanf("%s", b+1);     
    a[0] = '0';
    b[0] = '0';
    if (strlen(a) < strlen(b))
    {strcpy(tmp, a);
    strcpy(a, b);
    strcpy(b, tmp);}
    //set Table
    int len1, len2;
    len1 = strlen(a);
    len2 = strlen(b);
    table = (int**)malloc(sizeof(int*)*len2);
    for (int i = 0; i < len2; i++) {
        table[i] = (int*)malloc(sizeof(int)*len1);
    }
 
    for (int i = 0; i < len1; i++) {
        table[0][i] = 0;
    }
 
    //Calculation Table Index and LCS Length
    for (int i = 1; i < len2; i++) {
        max = 0;
        table[i][0] = 0;
        for (int j = 1;j < len1; j++) {
            if (b[i]== a[j]){
                max = table[i-1][j - 1] + 1;
                table[i][j] = max;
            }
            else {
                if(table[i][j - 1] > table[i - 1][j])
                    table[i][j] = table[i][j-1];
                else
                    table[i ][j] = table[i-1][j];
            }
        }
        if (LCS_length < max)
            LCS_length = max;
    }
    if (LCS_length==0){
    	printf("Not Match");
    	return 0;
	}
	else{
    int temp1, temp2, temp3, temp4;
    temp1 = LCS_length;
    temp2 = temp1 - 1;
    temp3 = len1 - 1;
    temp4 = LCS_length -1;
    char LCS[LCS_length];
    int count = 0;
    //Calculation LCS 
    for (int i = len2-1; i >0; i--) {
        for (int j = temp3; j > 0; j--) {
            if ((table[i][j] == temp1)&& table[i][j - 1] == temp2 && table[i - 1][j - 1] == temp2 && table[i - 1][j] == temp2 && table[i-1][j+1] != temp1) {
                temp2--;
                temp1--;
                LCS[temp4--] = b[i];
                temp3 = j;
                break;
            }
        }
    }
    printf("%s", LCS);
    return 0;}
}
