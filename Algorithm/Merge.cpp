// 2013012115_¿Ã∞«øµ 

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

void merge(int arr[], int temp[], int l_pos, int r_pos, int r_end){
	int i, l_end, num_elements, tmp_pos;
	l_end = r_pos - 1;
	tmp_pos = l_pos;
	num_elements = r_end - l_pos + 1;

	while ((l_pos <= l_end) && (r_pos <= r_end)){
		if (arr[l_pos] >= arr[r_pos]){
			temp[tmp_pos++] = arr[l_pos++];
		}
		else{
			temp[tmp_pos++] = arr[r_pos++];
		}
	}

	while (l_pos <= l_end){
		temp[tmp_pos++] = arr[l_pos++];
	}
	while (r_pos <= r_end){
		temp[tmp_pos++] = arr[r_pos++];
	}

	for (i = 0; i < num_elements; i++, r_end--){
		arr[r_end] = temp[r_end];
	}
}

void merge_sort(int arr[], int temp[], int left, int right){
	int center;
	if (left < right){
		center = (left + right) / 2;
		merge_sort(arr, temp, left, center);
		merge_sort(arr, temp, center + 1, right);
		merge(arr, temp, left, center + 1, right);
	}
}

int main(){
	int num, i;
	int* arr;
	int* temp;

	scanf("%d", &num);
	if ((num < 1) || (100000 < num)){
		printf("you can input a number (1<= number <= 100,000)\n");
		return 0;
	}

	arr = (int*)malloc(num * sizeof(int));
	temp = (int*)malloc(num * sizeof(int));

	for(i=0; i < num; i++){
		scanf("%d", &arr[i]);
	}

	merge_sort(arr, temp, 0, num-1);

	for (i = 0; i < num; i++){
		printf("%d\n", arr[i]);
	}

	free(arr);
	free(temp);

	return 0;
}
