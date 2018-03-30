// 2013012115_¿Ã∞«øµ 

#include <stdio.h>

void array_swap(int * arr, int a, int b){
	int temp;
	temp = arr[a];
	arr[a] = arr[b];
	arr[b] = temp;
}

void Heapify(int * arr, int parent_pos, int heap_size){
	int left, right, largest;

	left = 2 * parent_pos + 1;
	right = 2 * parent_pos + 2;

	if ((left < heap_size) && (arr[left] > arr[parent_pos])){
		largest = left;
	}
	else{
		largest = parent_pos;
	}

	if ((right < heap_size) && (arr[right] > arr[largest])){
		largest = right;
	}

	if (largest != parent_pos){
		array_swap(arr, parent_pos, largest);
		Heapify(arr, largest, heap_size);
	}
}

void Build_Heap(int * arr, int length){
	int parent_pos;

	for (parent_pos = length / 2 - 1; parent_pos >= 0; parent_pos--){
		Heapify(arr, parent_pos, length);
	}
}

void Heap_Sort(int * arr, int length, int k){
	Build_Heap(arr, length);

	int last_row;
	int count = 0;

	for (last_row = length - 1; last_row>0; last_row--){
		if (count == k){
			return;
		}
		array_swap(arr, 0, last_row);
		length--;
		count++;

		Heapify(arr, 0, length);
	}
}
int main(void){
	int arr[100000];
	int N, k, i;

	scanf("%d %d", &N, &k);

	if ((N < 1) || (N > 100000)){
		return 0;
	}
	else if ((k < 1) || (k > 30)){
		return 0;
	}

	for (i = 0; i < N; i++){
		scanf("%d", &arr[i]);
	}
	Heap_Sort(arr, N, k);

	for (i = N - 1; i > (N - 1) - k; i--){
		printf("%d ", arr[i]);
	}
	printf("\n");

	for (i = 0; i <= (N - 1) - k; i++){
		printf("%d ", arr[i]);
	}
	printf("\n");

	return 0;
}
