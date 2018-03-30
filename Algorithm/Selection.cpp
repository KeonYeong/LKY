// 2013012115_ÀÌ°Ç¿µ 

#include <stdio.h>
#include <stdlib.h>

void selection_sort(int arr[], int num, int step){
	int i, j, min, tmp;

	for (i = 0; i < step; i++){
		min = i;
		for (j = i + 1; j < num; j++){
			if (arr[min] > arr[j]){
				min = j;
			}
		}
		tmp = arr[i];
		arr[i]=arr[min];
		arr[min]=tmp;
		}
}

int main(){
	int num, step, i;
	int* arr;
	int* temp;

	scanf("%d %d", &num, &step);
	if ((num < 1) || (step < 1) || (30000 < num) || (30000 < step)){
		return 0;
	}

	arr = (int*)malloc(num * sizeof(int));

	for (i = 0; i < num; i++){
		scanf("%d", &arr[i]);
	}

	selection_sort(arr, num, step);

	for (i = 0; i < num; i++){
		printf("%d\n", arr[i]);
	}

	free(arr);

	return 0;
}
