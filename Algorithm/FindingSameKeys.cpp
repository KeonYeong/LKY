//2013012115_¿Ã∞«øµ 
#include <set>
#include <cstdio>
#include <iostream>
using namespace std;
int main (void){
	int N, M;
	int tmp;
	int findKeys = 0;
	set<int>s;
	scanf("%d %d\n", &N, &M);
	for(int i = 0 ; i < N; i ++){
		scanf("%d", &tmp);
		s.insert(tmp);
	}
	for(int i= 0 ; i < M ; i ++){
		scanf("%d", &tmp);
		if(s.find(tmp) != s.end())findKeys++;
	}
	printf("%d\n", findKeys);
	return 0;
}
