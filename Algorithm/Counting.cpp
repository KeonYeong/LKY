//2013012115_¿Ã∞«øµ 

#include <stdio.h>
#include <stdlib.h>

int main(){
	int N, M, K;
	int i;
	int *A;
	int *B;
	int *C;
	int *arr;

	scanf("%d %d %d", &N, &M, &K);

	A = (int *)malloc(sizeof(int)* K);
	B = (int *)malloc(sizeof(int)* K);
	C = (int *)malloc(sizeof(int)* (M+1));
	arr = (int *)malloc(sizeof(int)* N);

	for (i = 0; i <= M; i++){
		C[i] = 0;
	}

	for (i = 0; i < K; i++){
		scanf("%d %d", &A[i], &B[i]);
	}

	for (i = 0; i < N; i++){
		scanf("%d", &arr[i]);
		C[arr[i]] += 1;
	}

	for (i = 1; i <= M; i++){
		C[i] += C[i - 1];
	}

	for (i = 0; i < K; i++){
		printf("%d\n", C[B[i]] - C[A[i] - 1]);
	}

	free(A);
	free(B);
	free(C);
	free(arr);

	return 0;
}
