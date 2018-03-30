//2013012115_¿Ã∞«øµ
 
#include <stdio.h>
#include <stdlib.h>

int N;
int * list;
int * Qlist;
int * rep;

typedef struct _vertex;

typedef struct _vertex
{
	int groupname;
	int value;
	int rank;
	struct _vertex * parent;
} Vertex;

Vertex * vertex;

void MAKE_SET(Vertex * x, int value)
{
	x->value = value;
	x->parent = x;
	x->rank = 0;
}

Vertex * FIND_SET(Vertex * x)
{
	if (x != x->parent)
		x->parent = FIND_SET(x->parent);
	return x->parent;
}

void LINK(Vertex * x, Vertex * y)
{
	if (x->rank > y->rank){
		y->parent = x;
	}
	else{
		x->parent = y;
		if (x->rank == y->rank){
			y->rank = y->rank + 1;
		}
	}
}

void UNION(Vertex * x, Vertex * y)
{
	LINK(FIND_SET(x), FIND_SET(y));
}

int main(void)
{
	int * tmp_list;
	int v1, v2, i = 0, j = 0, tmp = 0, order =0, compNUM = 0;

	scanf("%d", &N);
	vertex = (Vertex*)malloc(sizeof(Vertex)*(N + 1));
	list = (int*)malloc(sizeof(int)*(N + 1));
	tmp_list = (int*)malloc(sizeof(int)*(N + 1));

	for (i = 1; i <= N; i++)
		MAKE_SET(&vertex[i], i);

	while (scanf("%d %d", &v1, &v2) != EOF){
		if (FIND_SET(&vertex[v1]) != FIND_SET(&vertex[v2])){
			UNION(&vertex[v1], &vertex[v2]);
		}
	}
	for (i = 1; i <= N; i++){
		list[i] = FIND_SET(&vertex[i])->value;
		tmp_list[i] = FIND_SET(&vertex[i])->value;
	}

	for (i = 1; i <= N; i++)
	{
		tmp = list[i];
		if (tmp != 0)
		{
			compNUM++;
			for (j = i + 1; j <= N; j++){
				if (tmp == list[j])
					list[j] = 0;
			}
		}
	}
	Qlist = (int*)malloc(sizeof(int)*(compNUM + 1));
	order = 1;
	for (i = 1; i <= N; i++){
		if (list[i] != 0){
			Qlist[order++] = list[i];
		}
	}
	for (i = 1; i <= N; i++){
		for (j = 1; j <= N; j++){
			if (tmp_list[i] == Qlist[j]){
				list[i] = j;
			}
		}
	}

	printf("%d\n", compNUM);
	for (i = 1; i <= N; i++)
		printf("%d\n", list[i]);

	return 0;
}
