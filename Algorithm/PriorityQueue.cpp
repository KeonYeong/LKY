// 2013012115_ÀÌ°Ç¿µ 

#include <stdio.h>

void array_swap(int arr[], int a, int b){
	int temp;
	temp = arr[a];
	arr[a] = arr[b];
	arr[b] = temp;
}

void Heapify(int arr[], int current, int length){
	int left, right, largest;
	left = current * 2;
	right = current * 2 + 1;

	if ((right <= length) && (arr[right] > arr[current])){
		largest = right;
	}
	else{
		largest = current;
	}

	if ((left <= length) && (arr[left] > arr[largest])){
		largest = left;
	}

	if (largest != current){
		array_swap(arr, current, largest);
		Heapify(arr, largest, length);
	}
}

void Increase_key(int arr[], int index, int key){
	while (index > 1 && arr[index / 2] < arr[index]){
		array_swap(arr, index, index / 2);

		index /= 2;
	}
}

int main(void){
	int arr[100000];
	int extracted[100000];
	int check, i, num, index, length = 0, ext_length = 0;

	while (1){
		scanf("%d", &check);

		switch (check){
		case 0:
			for (i = 1; i <= ext_length; i++){
				printf("%d ", extracted[i]);
			}
			printf("\n");

			for (i = 1; i <= length; i++){
				printf("%d ", arr[i]);
			}

			return 0;

		case 1:
			scanf("%d", &num);
			length++;
			arr[length] = num;
			Increase_key(arr, length, num);
			break;

		case 2:
			extracted[++ext_length] = arr[1];

			array_swap(arr, 1, length);

			Heapify(arr, 1, --length);
			break;

		case 3:
			scanf("%d %d", &index, &num);
			arr[index] = num;
			if (index != 1 && arr[index] > arr[index / 2]){
				Increase_key(arr, index, num);
			}
			else{
				Heapify(arr, index, length);
			}
			break;

		default:
			break;
		}
	}
}
