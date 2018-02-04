#include <iostream>
#include <unordered_set>
#include <string>
#include <map>
#include <queue>
#include <thread>
#include <pthread.h>
using namespace std;
const int NUM_THREAD = 36; // 코어 개수의 따른 스레드 개수 조정
struct Trie { // 아호코라식에서의 나무 구조체
	Trie *edges[26];
	Trie *fail; // 검색실패시 백업노드
	unordered_set<string> out;  // 각 노드가 들고 있는 출력값
	bool changed;
	Trie() {
		fail = NULL;
		changed = false;
		for (int i = 0; i < 26; i++) {
			edges[i] = NULL;
		}
	}
};
struct threadData{ // 각 스레드가 쓰게 될 데이터, 아이디에 따라 분류
	unordered_set<string> words;
	bool allReady;
	bool updated;
};
pthread_cond_t cond= PTHREAD_COND_INITIALIZER;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
multimap<pair<size_t, size_t>, string> resultMap; // 결과가 저장될 멀티맵
threadData forThread[NUM_THREAD];
bool noMoreWork = false;
unordered_set<string> word_list;
int upcomingThread = 0;
int jobDone = 0;
string str;
bool deleted;
void addString(Trie *node, string curString) { //최초에 나무 구조를 생성하는 함수
	int next;
	int depth = 0;
	while (depth != curString.length()) {
		int next = curString[depth] - 'a';

		if (node->edges[next] == NULL || !node->edges[next]->changed) {
			node->edges[next] = new Trie();
			node->edges[next]->changed = true;
		}
		node = node->edges[next];
		depth = depth + 1;
	}
	node->out.insert(curString);
}
void* ThreadFunc(void * arg){ // 스레드의 함수, 나무를 만들고, fail링크를 만들며 검색하여 결과 멀티맵에 집어넣는다.
	long tid = (long)arg;
	Trie * root = new Trie();
	queue<Trie*> q;
	forThread[tid].allReady = true;
	pthread_mutex_lock(&mutex);
	pthread_cond_wait(&cond, &mutex);
	pthread_mutex_unlock(&mutex);
	bool firstTime = true;
	bool detected = false;
	while(!noMoreWork){
		if(!forThread[tid].words.empty()){
		if(forThread[tid].updated||firstTime||deleted){
			root = new Trie();
			for (int i = 0 ; i < 26 ; i ++){
				root->edges[i] = root;
				root->edges[i]->changed = false;
			}
			for (unordered_set<string>::iterator it = forThread[tid].words.begin(); it != forThread[tid].words.end(); it++) addString(root, (*it));

			root->fail = root;
			for (int i = 0; i < 26; i++) {
				if (root->edges[i] != NULL && root->edges[i] != root) {
					root->edges[i]->fail = root;
					q.push(root->edges[i]);
				}
			}
			while (!q.empty()) {
				Trie *curNode = q.front();
				q.pop();
				for (int i = 0; i < 26; i++) {
					Trie *next = curNode->edges[i];
					if (next != NULL && next != root) {
						q.push(next);
						Trie *f = curNode->fail;
						for (; f->edges[i] == NULL; f = f->fail);
						next->fail = f->edges[i];
						for (unordered_set<string>::iterator it = next->fail->out.begin(); it != next->fail->out.end(); it ++) next->out.insert(*it);
					}
				}
			}
			firstTime = false;
			forThread[tid].updated = false;
		}
		Trie * node = root;
		for (int i = 0; i < str.size() ; i++) {
			int cur = str[i] - 'a';
			for (; node->edges[cur] == NULL; node = node->fail);
			node = node->edges[cur];
			if (node->out.size() != 0) {
				pthread_mutex_lock(&mutex);
				for (unordered_set<string>::iterator it = node->out.begin(); it != node->out.end(); it++){
						for (multimap<pair<size_t,size_t>,  string>::iterator it2 = resultMap.begin(); it2 != resultMap.end(); it2++){
						if (it2->second == (*it)){
							detected = true;
							break;
						}
					}
					if(!detected) resultMap.insert(make_pair(make_pair(i+1-(*it).length(), i+1), (*it)));
					detected = false;
				}
				pthread_mutex_unlock(&mutex);
			}
		}
		}
		pthread_mutex_lock(&mutex);
		jobDone ++;
		pthread_cond_wait(&cond,&mutex);
		pthread_mutex_unlock(&mutex);
	}
}
int main() {
	pthread_t threads[NUM_THREAD];
	int N;
	char M;
	for(int i = 0; i < NUM_THREAD; i ++)forThread[i].allReady = false;	
	for (long i = 0 ; i < NUM_THREAD; i++){
		if (pthread_create(&threads[i], 0, ThreadFunc, (void*)i) < 0) {
			cout << "pthread_create error!" << endl;
			return 0;
		}
		forThread[i].updated = false;
		while (!forThread[i].allReady){
			pthread_yield();
		}
	}
	std::ios::sync_with_stdio(false);
	cin >> N;
	for(int i = 0; i < N; i++){
		cin >> str;
		word_list.insert(str);
	}
	cout << "R" << std::endl;
	bool initTrie = true;
	deleted = false;
	while(cin >> M){
		cin.get();
		getline(cin, str);
		switch(M){
			case 'Q': // 쿼리가 입력됬을 시 스레드를 깨우고 작업을 실행시킨다.
				{ 
					if(initTrie||deleted){
						unordered_set<string>::iterator itDivide = word_list.begin();
						int nowThread = 0;
						for (int cnt = 1 ; itDivide != word_list.end(); cnt++, itDivide++){
							forThread[nowThread].words.insert(*itDivide);
							nowThread = (nowThread + 1) % NUM_THREAD;
						}
						upcomingThread = nowThread;
						initTrie = false;
					}
					resultMap.clear();
					pthread_mutex_lock(&mutex);
					pthread_cond_broadcast(&cond);
					pthread_mutex_unlock(&mutex);
					while(1){
						if(jobDone != NUM_THREAD) pthread_yield();
						else break;
					}
					jobDone = 0;
					deleted = false;
					if(!resultMap.empty()){
						multimap<pair<size_t, size_t>, string>::iterator itOp = resultMap.begin();
						for (int cnt = resultMap.size(); cnt != 0; cnt--, itOp++){
							cout << itOp->second;
							if(cnt != 1) cout<<"|";
						}
					}
					else cout << -1 ;
					cout << std::endl;
				}
				break;
			case 'A' : // 입력받을 때마다 본인의 단어리스트에 추가해놓고 마찬가지로 스레드의 단어리스트에도 추가한다.
				{
					if(word_list.find(str) != word_list.end())break;
					word_list.insert(str);
					forThread[upcomingThread].words.insert(str);
					forThread[upcomingThread].updated = true;
					upcomingThread = (upcomingThread+1) % NUM_THREAD;
				}
				break;
			case 'D' : // 제거할 시엔 메인 단어리스트는 물론 스레드의 단어리스트에 있는지 검색하여 삭제한다.
				{
					if(word_list.find(str) == word_list.end())break;
					for(int i = 0; i < NUM_THREAD; i ++){
						if(forThread[i].words.find(str) != forThread[i].words.end()){
							forThread[i].words.erase(str);
							forThread[i].updated = true;
							if(forThread[i].words.empty())deleted = true;
							upcomingThread = i;
							break;
						}
					}
					word_list.erase(str);
				}
				break;
		}
	}
	return 0;
}
