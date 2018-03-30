// 2013012115_¿Ã∞«øµ 

#include <stdio.h>
#include <stdlib.h>

int main(){
	int i, N, E1, E2, X1, X2;
	int *station1;
	int *station2;
	int *trans1;
	int *trans2;
	int *price1;
	int *price2;
	int **from;
	int *path;

	scanf("%d", &N);
	scanf("%d %d", &E1, &E2);
	scanf("%d %d", &X1, &X2);

	station1 = (int *)malloc(sizeof(int)*(N + 1));
	station2 = (int *)malloc(sizeof(int)*(N + 1));
	trans1 = (int *)malloc(sizeof(int)*N);
	trans2 = (int *)malloc(sizeof(int)*N);
	price1 = (int *)malloc(sizeof(int)*(N + 1));
	price2 = (int *)malloc(sizeof(int)*(N + 1));
	from = (int **)malloc(sizeof(int*)* 2);
	from[0] = (int *)malloc(sizeof(int)*(N + 1));
	from[1] = (int *)malloc(sizeof(int)*(N + 1));
	path = (int *)malloc(sizeof(int)*(N + 1));

	for (i = 1; i <= N; i++){
		scanf("%d", &station1[i]);
	}
	for (i = 1; i <= N; i++){
		scanf("%d", &station2[i]);
	}
	for (i = 1; i < N; i++){
		scanf("%d", &trans1[i]);
	}
	for (i = 1; i < N; i++){
		scanf("%d", &trans2[i]);
	}

	from[0][1] = 1;
	from[1][1] = 2;

	price1[1] = E1 + station1[1];
	price2[1] = E2 + station2[1];
	for (i = 2; i <= N; i++){
		if (price1[i - 1] <= price2[i - 1] + trans2[i - 1]){
			from[0][i] = 1;
			price1[i] = station1[i] + price1[i - 1];
		}
		else{
			from[0][i] = 2;
			price1[i] = station1[i] + price2[i - 1] + trans2[i - 1];
		}

		if (price2[i - 1] <= price1[i - 1] + trans1[i - 1]){
			from[1][i] = 2;
			price2[i] = station2[i] + price2[i - 1];
		}
		else{
			from[1][i] = 1;
			price2[i] = station2[i] + price1[i - 1] + trans1[i - 1];
		}
	}

	if (price1[N] + X1 <= price2[N] + X2){
		printf("%d\n", price1[N] + X1);
		path[N] = 1;
	}
	else{
		printf("%d\n", price2[N] + X2);
		path[N] = 2;
	}

	for (i = N; i > 1; i--){
		path[i - 1] = from[path[i] - 1][i];
	}
	for (i = 1; i <= N; i++){
		printf("%d %d\n", path[i], i);
	}

	free(price1);
	free(price2);
	free(from[0]);
	free(from[1]);
	free(from);
	free(station1);
	free(station2);
	free(trans1);
	free(trans2);
	free(path);

	return 0;
}
