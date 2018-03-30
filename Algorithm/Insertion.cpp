//2013012115_ÀÌ°Ç¿µ 

#include <stdio.h>
#include <stdlib.h>

void insertion_sort(int arr[], int num){

	int i, j, temp;
	for (i = 1; i < num; i++)
	{
		temp = arr[(j = i)];
		while ((--j >= 0) && (temp > arr[j])){
			arr[j + 1] = arr[j];
		}
		arr[j + 1] = temp;
	}
}

int main(){
	int num, input, i;
	int* arr;

	scanf("%d", &num);
	if ((num < 1) || (30000 < num)){
		return 0;
	}

	arr = (int *)malloc(num * sizeof(int));

	for (i = 0; i < num; i++){
		scanf("%d", &arr[i]);
	}

	insertion_sort(arr, num);

	for (i = 0; i < num; i++){
		printf("%d\n", arr[i]);
	}

	free(arr);

	return 0;
}
