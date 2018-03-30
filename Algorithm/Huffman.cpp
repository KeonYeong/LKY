//2013012115_¿Ã∞«øµ 

#include<stdio.h>
#include<stdlib.h>

typedef struct Element {
	int value;
	struct Element *left_child;
	struct Element *right_child;
}Element;

typedef struct HeapType {
	Element heap[30010];
	int heap_size;
}HeapType;

Element et[35000];
int huff_size = 0;

void Insert(HeapType *H, Element e) {
	int i;
	i = ++(H->heap_size);

	while ((i != 1) && (e.value < H->heap[i / 2].value)) {
		H->heap[i] = H->heap[i / 2];
		i = i / 2;
	}

	H->heap[i] = e;
}

void CreateHeap(HeapType *H, int size) {
	int i;

	H->heap_size = 0;

	for (i = 0; i < size; i++) {
		if (et[i].value) {
			Insert(H, et[i]);
		}
	}
}

Element *Delete(HeapType *H) {
	int parent = 1, child = 2;
	Element *e = (Element*)malloc(sizeof(Element));
	Element tmp;

	*e = H->heap[parent];
	tmp = H->heap[(H->heap_size)--];

	while (child <= H->heap_size) {
		if ((child < H->heap_size) && (H->heap[child].value > H->heap[child + 1].value)) {
			child++;
		}

		if (tmp.value < H->heap[child].value) {
			break;
		}

		H->heap[parent] = H->heap[child];
		parent = child;
		child *= 2;
	}

	H->heap[parent] = tmp;

	return e;
}

Element *HuffmanTree(HeapType *H) {

	Element *e1, *e2;
	Element tmp;

	while (H->heap_size > 1) {
		e1 = Delete(H);
		e2 = Delete(H);
		tmp.value = e1->value + e2->value;
		tmp.left_child = e1;
		tmp.right_child = e2;
		Insert(H, tmp);
	}

	return &(H->heap[1]);
}

void CheckHuff(Element *e, int size) {

	if (e->left_child) {
		CheckHuff(e->left_child, size + 1);
	}
	if (e->right_child) {
		CheckHuff(e->right_child, size + 1);
	}

	if (!(e->left_child) && !(e->right_child)) {
		huff_size += e->value * size;
	}
}

int main() {
	int N, num, S, i, k = 0;
	char name[5];
	HeapType H;
	Element *e;

	scanf("%d", &N);

	for (i = 0; i < N; i++) {
		scanf("%s %d", name, &num);
		et[i].value = num;
		et[i].left_child = NULL;
		et[i].right_child = NULL;
	}

	scanf("%d", &S);

	CreateHeap(&H, N);

	e = HuffmanTree(&H);

	CheckHuff(e, 0);

	N--;

	while (N > 0) {
		k++;
		N /= 2;
	}

	if (k == 0) {
		k++;
	}

	printf("%d\n%d\n", k * S, huff_size);

	return 0;
}
