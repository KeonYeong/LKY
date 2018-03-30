//2013012115_¿Ã∞«øµ 

#include <iostream>
#include <vector>
#include <queue>
#include <functional>
#include <algorithm>
#define INF 100000

using namespace std;

priority_queue<pair<int, int>, vector<pair<int, int> >, greater<pair<int, int> > > myQ;
vector<pair<int, int> > node[5001];
int N, M;
int K;
int u, v, w;

int dijkstra(int st)
{
	int length[5001];
	for (int i = 1; i <= N; i++)
	{
		if (i != st)
			length[i] = INF;
		else
			myQ.push(make_pair(length[i], i));
	}
	length[st] = 0;
	while (!myQ.empty())
	{
		int u = myQ.top().second, val = myQ.top().first;
		myQ.pop();
		for (int j = 0; j < node[u].size(); j++)
		{
			if (length[u] + node[u][j].second < length[node[u][j].first])
			{
				length[node[u][j].first] = length[u] + node[u][j].second;
				myQ.push(make_pair(length[node[u][j].first], node[u][j].first));
			}
		}
	}
	int MAX = 0;
	for (int i = 1; i <= N; i++) {
		MAX = max(MAX, length[i]);
	}
	return MAX;
}

int main()
{
	cin >> N;
	for (int i = 1; i <= N; i++) {
		int temp, num;
		cin >> temp >> num;
		for (int j = 0; j < num; j++) {
			int v, w;
			cin >> v >> w;
			node[i].push_back(make_pair(v, w));
		}
	}
	cout << dijkstra(1);
	return 0;
}
