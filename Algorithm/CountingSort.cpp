//2013012115_¿Ã∞«øµ
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct _vertex{
	int color;
	int f;
	int d;
} Vertex;

typedef struct _VertexInfo{
	int out_degree;
	int * list;
} VertexInfo;

Vertex * vertex;
VertexInfo * vertexInfo;
int numV, time;
int * topoList;
int topo_num = 0;
int isDAG = 1;

void addEdge(int fromV, int toV){
	int i = 0;
	int cur = 0;

	for (i = 0; i < numV; i++)
	{
		if (vertexInfo[fromV].list[i] == 0)
			break;
		cur++;
	}
	vertexInfo[fromV].list[cur] = toV;
}

void DFS_Visit(int fromV){
	int i = 0, toV;

	time++;
	vertex[fromV].color = 1;
	vertex[fromV].d = time;

	for (i = 0; i < vertexInfo[fromV].out_degree; i++){
		toV = vertexInfo[fromV].list[i];
		switch (vertex[toV].color)
		{
		case 0:
			DFS_Visit(toV);
			break;

		case 1:
			isDAG = 0;
			break;

		case 2:
			break;
		}
	}

	vertex[fromV].color = 2;
	topoList[numV - (topo_num++)] = fromV;
	time++;
	vertex[fromV].f = time;
}

void Sort(int list[], int num)
{
	int i = 0, j = 0, tmp;

	for (i = 0; i < num; i++)
	{
		for (j = i + 1; j < num; j++)
		{
			if (list[i] > list[j])
			{
				tmp = list[i];
				list[i] = list[j];
				list[j] = tmp;
			}
		}
	}
}


void DFS(void)
{
	int i;
	for (i = 1; i <= numV; i++)
	{
		vertex[i].d = 0;
		vertex[i].f = 0;
		vertex[i].color = 0;
		Sort(vertexInfo[i].list, vertexInfo[i].out_degree);
	}

	time = 0;
	for (i = 1; i <= numV; i++)
	{
		if (vertex[i].color == 0)
		{
			DFS_Visit(i);
		}
	}
}


int main(void)
{
	int i = 0, j = 0;
	int fromV, toV;

	scanf("%d", &numV);

	vertexInfo = (VertexInfo*)malloc(sizeof(VertexInfo)*(numV + 1));
	for (i = 1; i <= numV; i++){
		vertexInfo[i].list = (int*)malloc(sizeof(int)*numV);
		vertexInfo[i].out_degree = 0;
	}
	for (i = 1; i <= numV; i++){
		for (j = 0; j < numV; j++){
			vertexInfo[i].list[j] = 0;
		}
	}
	vertex = (Vertex*)malloc(sizeof(Vertex)*(numV + 1));
	topoList = (int*)malloc(sizeof(int)*(numV + 1));


	while (scanf("%d %d", &fromV, &toV) != EOF){
		addEdge(fromV, toV);
		vertexInfo[fromV].out_degree++;
	}
	DFS();

	printf("%d\n", isDAG);
	if (isDAG)
	{
		for (i = 1; i <= numV; i++)
			printf("%d ", topoList[i]);
	}

	return 0;
}
