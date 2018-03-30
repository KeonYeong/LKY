// 2013012115_¿Ã∞«øµ 

#include <stdio.h>
#include <stdlib.h>

void order(int i, int j, int **S){
	int k;

	if (i == j){
		printf("%d", i);
	}
	else{
		k = S[i][j];
		printf("(");
		order(i, k, S);
		order(k + 1, j, S);
		printf(")");
	}
}

int MinMult(int N, int *p, int **M, int **S){
	int i, j, k, final_k, diagonal, min;

	for (diagonal = 1; diagonal <= N - 1; diagonal++){
		for (i = 1; i <= N - diagonal; i++){
			j = i + diagonal;
			min = 1000000;
			for (k = i; k <= j - 1; k++){
				if (min > M[i][k] + M[k + 1][j] + p[i - 1] * p[k] * p[j]){
					min = M[i][k] + M[k + 1][j] + p[i - 1] * p[k] * p[j];
					final_k = k;
				}
			}
			M[i][j] = min;
			S[i][j] = final_k;
		}
	}
	return M[1][N];
}


int main(){

	int i, N;
	int *p;
	int **M;
	int **S;

	scanf("%d", &N);

	p = (int *)malloc(sizeof(int)*(N + 1));
	M = (int **)malloc(sizeof(int *)*(N + 1));
	S = (int **)malloc(sizeof(int *)*(N + 1));
	for (i = 0; i < N; i++){
		M[i] = (int *)malloc(sizeof(int)*(N + 1));
		S[i] = (int *)malloc(sizeof(int)*(N + 1));

		M[i][i] = 0;
	}
	M[N] = (int *)malloc(sizeof(int)*(N + 1));
	M[N][N] = 0;

	for (i = 0; i <= N; i++){
		scanf("%d", &p[i]);
	}

	printf("%d\n", MinMult(N, p, M, S));
	order(1, N, S);

	free(p);
	for (i = 0; i < N; i++){
		free(M[i]);
		free(S[i]);
	}
	free(M[N]);
	free(M);
	free(S);

	return 0;
}
