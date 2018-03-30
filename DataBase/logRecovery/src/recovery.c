#define Version "1.14"
#include "recovery.h"
// GLOBALS.
ulint currentOff = 0; // 현재 파일이 몇번째 offset인지 
ulint addedOff = 0;	// insert시 새로 생기게 되는 페이지의 offset
ulint remainingOff = 0;	// delete 시 병합 후 남게 되는 페이지의 offset
page* buffPage;	// 현재 파일을 담고 있는 캐쉬상 데이터
page* neighbor;	// delete시 이웃 노드를 알기 위한 페이지포인터
char retBuff[120];	// find 반환 시 데이터 손실을 막기 위한 임시 캐릭터 배열
Buffer* buffTable;  // 실질적인 버퍼 풀, 여기에 페이지와 각종 컨트롤 비트가 들어가게 된다.
int buffNum;    // 총 몇개의 버퍼인지 알려주는 변수
Table * tables; // 파일을 열 때 그 파일의 간단한 정보를 담고 있는 테이블이다.
int clockHand;  //방출 하는 policy는 clock algorithm 을 채택하였기에, 그에 따른 clock이 가르키는 현 인덱스이다.
int currentInBuff;  // 현재 버프에 몇개의 페이지들이 들어있는 지 알려주는 변수
int bufferOn = 1;   // 이 변수가 0이 될경우 버퍼를 적용하지 않고 바로 파일에 쓰게 된다.
int usingIndex;
Log * logBuffer;
Xact xInfo;
int curLog = 0;
ulint flushedLSN = 0;
int log_fd;
int vocal = 0;

void gv_init(){		// 전역변수들의 초기화
    currentOff = 0;
    addedOff = 0;
    remainingOff = 0;
    usingIndex = 0;
    memset(buffPage, 0, sizeof(page));
    memset(neighbor, 0, sizeof(page));
}

void fileRead(ulint offset, page* buff, int fd){
    lseek(fd, offset, SEEK_SET);
    read(fd, buff, sizeof(page));
}

void fileWrite(ulint offset, page* buff, int fd){
    lseek(fd, offset, SEEK_SET);
    write(fd, buff, sizeof(page));
}

// 입출력 중에 입력을 하는 함수, 버퍼를 통해서 실행하게 됨
int bufferRead(ulint offset, page* buff, int table_id, int copy){
    if(bufferOn==0){    // 만일 버퍼를 사용하지 않는다고 하면 곧바로 디스크 입출력을 한다.
        fileRead(offset, buff, tables[table_id - 1].fd);
        return -1;
    }
    int i;
    for(i = 0 ; i < buffNum; i ++){ 
        if(buffTable[i].table_id == table_id && buffTable[i].pageOff == offset){    // 선형으로 검색하며 만약 버퍼 풀에 페이지가 존재할 경우 해당 페이지를 리턴한다.
            if(copy) memcpy(buff, &buffTable[i].frame, PGSIZE);
            break;
        }
    }
    if(i == buffNum){ // 만약 버퍼 풀에 해당 페이지가 존재하지 않는다면 적절한 policy로 하나의 페이지를 비우고 거기에 새로운 페이지를 디스크에서 읽어오게된다.
        int idx;
        if(currentInBuff == buffNum) idx = selectVictim(table_id);  // 한 페이지를 골라서 버린다.
        else {
            for(i = 0 ; i < buffNum; i ++){     // 꽉 찬게 아닐 경우 페이지를 새로 버퍼에 추가한다.
                if(buffTable[i].table_id == 0){
                    idx = i;
                    break;
                }
            }
        }
        frame_init(idx, table_id, offset);  // 이제 자리가 났으므로 초기화 시켜주고
        fileRead(offset, &buffTable[idx].frame, tables[table_id - 1].fd);  // 그 자리에 해당 페이지를 디스크에서 읽어온다.
        currentInBuff++;   
        if(copy) memcpy(buff, &buffTable[idx].frame, PGSIZE);    // 그 페이지를 반환.
        return idx;
    }
    else return i;
}

int bufferWrite(ulint offset, page* buff, int table_id){
    if(bufferOn == 0){  //하는 일은 bufferRead에서 하는 일과 완벽히 똑같다. 
        fileWrite(offset, buff, tables[table_id - 1].fd);
        return -1;
    }
    int i;
    for(i = 0; i < buffNum; i ++){
        if(buffTable[i].table_id == table_id && buffTable[i].pageOff == offset){
            memcpy(&buffTable[i].frame, buff, PGSIZE);  // bufferRead와 달리 페이지를 버퍼에 쓰고 종료시킨다.
            buffTable[i].is_dirty = 1;
            break;
        }
    }
    if(i == buffNum){
        int idx;
        if(currentInBuff == buffNum) idx = selectVictim(table_id);
        else {
            for(i = 0 ; i < buffNum; i ++){
                if(buffTable[i].table_id == 0){
                    idx = i;
                    break;
                }
            }
        }
        frame_init(idx, table_id, offset);
        fileRead(offset, &buffTable[idx].frame, tables[table_id - 1].fd);
        currentInBuff++;
        memcpy(&buffTable[idx].frame, buff, PGSIZE);    // 이부분이 bufferRead와 다른데, 여기선 페이지를 오히려 버퍼에 쓰고 종료시키게 된다.
        buffTable[idx].is_dirty = 1;    // is_dirty를 켜준다.
        return idx;
    }
    else return i;
}

// 페이지를 비우는 policy로써 현재는 clock algorithm이 적용되어 있다.
int selectVictim(int table_id){
    int victim; // 버리게 될 버퍼의 인덱스
    while(1){
        if(buffTable[clockHand].pin_count == 0 && buffTable[clockHand].table_id != 0){  // 버퍼내의 pin_count 가 0이면서, 비어있는 버프가 아닐경우
            if(!buffTable[clockHand].LRU){  // 해당 버퍼의 reference bit가 켜져있는지 본다.
                buffTable[clockHand].LRU = 1;   // 켜져있다면 그것을 내리고 다음 버퍼로 넘어간다.
                clockHand = (clockHand + 1) % buffNum;
            }
            else{   // 버릴 버퍼를 찾았을 경우다
                if(buffTable[clockHand].is_dirty) {
                    if(buffTable[clockHand].frame.pageLSN > flushedLSN && curLog != 0) log_flush(); //WAL policy
                    fileWrite(buffTable[clockHand].pageOff, &buffTable[clockHand].frame, tables[buffTable[clockHand].table_id - 1].fd);   // 우선 is_dirty인지 확인하여 디스크에 쓸 필요가 있는 지 확인한다. 필요할 경우 디스크에 쓴다.
                }
                frame_init(clockHand, 0, 0);    // 초기화 시켜주고
                currentInBuff--;
                victim = clockHand;
                clockHand = (clockHand + 1) % buffNum;  // clock_hand는 일단 다음 단계로 넘겨놓고
                return victim;  // 버린 그 버퍼의 인덱스를 반환한다.
            }
        }
        else{
            clockHand = (clockHand + 1) % buffNum;  // 아닐경우 clockHand만 한개 증가시킨다.
        }
    }
}

// 버퍼에 조인할 테이블들을 올리는 함수, 입력받은 버퍼 프레임 넘버에다가 해당 페이지를 로드해서 올린다.
int fetchPage(ulint pageOff, int frameNo, int table_id){
    fileRead(pageOff, &buffTable[frameNo].frame, tables[table_id - 1].fd);
// 기본 변수들 몇개만 변경해준다.
    buffTable[frameNo].table_id = table_id;
    buffTable[frameNo].is_dirty = 0;
    return 0;
}

// 조인 그중에서도 sort-merge를 준비하는 함수, 일단 각 파일의 b+ tree의 제일 왼쪽 노드를 찾아 들어가서 버퍼 0, 1 에 각각 하나씩 올린다. 그 후 버퍼 2 를 output buffer로 사용하기 위해 초기화시키게 된다.
void initSM(int table_id_1, int table_id_2, int buffers){
// 첫 기준이 되는 파일
    fetchPage(tables[table_id_1 - 1].header.rootPageOff, 0, table_id_1);
    while(!buffTable[0].frame.isLeaf){
        fetchPage(buffTable[0].frame.extraOff, 0, table_id_1);
    }
// 조인시키는 파일
    fetchPage(tables[table_id_2 - 1].header.rootPageOff, 1, table_id_2);
    while(!buffTable[1].frame.isLeaf){
        fetchPage(buffTable[1].frame.extraOff, 1, table_id_2);
    }
   memset(buffTable[2].join, 0, sizeof(Buffer));
  buffTable[2].join[0] = '\0'; 
}

int join_table(int table_id_1, int table_id_2, char* pathname){
    int i, j, mark;
    int joiner;
    if ((joiner = open(pathname, O_WRONLY | O_CREAT | O_EXCL | O_SYNC, 0644)) == -1) {
        if((joiner = open(pathname, O_WRONLY | O_SYNC)) == -1) return -1;
    }
    initSM(table_id_1, table_id_2, 1);
    i = 0;
    j = 0;
    while(1){
        if(i == buffTable[0].frame.numKeys){
            if(buffTable[0].frame.extraOff != 0){
                fetchPage(buffTable[0].frame.extraOff, 0, table_id_1);
                i = 0;
            }
            else {
                writePage(joiner);
                return 0;
            }
        }
        while(buffTable[0].frame.rec.lRec[i].key < buffTable[1].frame.rec.lRec[j].key) {
            i++;
            if(i == buffTable[0].frame.numKeys) {
                if(buffTable[0].frame.extraOff != 0){
                    fetchPage(buffTable[0].frame.extraOff, 0, table_id_1);
                    i = 0;
                }
                else {
                    writePage(joiner);
                    return 0;
                }
            }
        }
        while(buffTable[0].frame.rec.lRec[i].key > buffTable[1].frame.rec.lRec[j].key) {
            j++;
            if(j == buffTable[1].frame.numKeys) {
                if(buffTable[1].frame.extraOff != 0){
                    fetchPage(buffTable[1].frame.extraOff, 1, table_id_2);
                    j = 0;
                }
                else {
                    writePage(joiner);
                    return 0;
                }
            }
        }
        mark = j;
        while(buffTable[0].frame.rec.lRec[i].key == buffTable[1].frame.rec.lRec[j].key){
            while(buffTable[0].frame.rec.lRec[i].key == buffTable[1].frame.rec.lRec[j].key){
                joinOut(i, j, joiner);
                j++;
            }
            j = mark;
            i++;
        }
    }
}

void joinOut(int st, int nd, int fd){
    char result[300];
    sprintf(result, "%ld,%s,%ld,%s\n", buffTable[0].frame.rec.lRec[st].key, buffTable[0].frame.rec.lRec[st].value, buffTable[1].frame.rec.lRec[nd].key, buffTable[1].frame.rec.lRec[nd].value);
    if(strlen(buffTable[2].join)+strlen(result) >= 4120) writePage(fd);
    strcat(buffTable[2].join, result);
}

void writePage(int fd){
    write(fd, buffTable[2].join, strlen(buffTable[2].join));
    memset(buffTable[2].join, 0, sizeof(Buffer));
    buffTable[2].join[0] = '\0';
}

void appointFrame(int idx){
    buffTable[idx].pin_count++;
}

void terminateFrame(int idx){
    buffTable[idx].pin_count--;
}

void frame_init(int idx, int table_id, ulint offset){
    memset(&buffTable[idx].frame, 0, PGSIZE);
    buffTable[idx].table_id = table_id;
    buffTable[idx].pageOff = offset;
    buffTable[idx].is_dirty = 0;
    buffTable[idx].pin_count = 0;
    buffTable[idx].LRU = 0;
}

void log_update(int table_id, page* oldImg, int64_t key, char* value, ulint pageOff, int type){
    int r_idx;
    if(curLog == 0) logBuffer[curLog].PrevLSN = flushedLSN;
    else logBuffer[curLog].PrevLSN = logBuffer[curLog - 1].LSN;
    logBuffer[curLog].xid = xInfo.xid;
    logBuffer[curLog].type = type;
    logBuffer[curLog].LSN = logBuffer[curLog].PrevLSN + LOGSIZE;
    switch(type){
        case BEGIN:
            xInfo.initLSN = logBuffer[curLog].LSN;
            break;
        case UPDATE:
            r_idx = ((pageOff % PGSIZE) - PGHEADSIZE) / LRECSIZE;
            if(key == 1513){
     //           printf("page?: key %ld, value %s pageLSN %ld r_idx %d other key value %d %s numKey %d\n", oldImg->rec.lRec[r_idx].key, oldImg->rec.lRec[r_idx].value, oldImg->pageLSN, r_idx, oldImg->rec.lRec[2].key, oldImg->rec.lRec[2].value, oldImg->numKeys);
            }
            logBuffer[curLog].table_id = table_id;
            logBuffer[curLog].PGN = pageOff / PGSIZE;
            logBuffer[curLog].offset = pageOff % PGSIZE;
            logBuffer[curLog].length = strlen(oldImg->rec.lRec[r_idx].value) + 1;
            logBuffer[curLog].oImg.key = oldImg->rec.lRec[r_idx].key;
            logBuffer[curLog].nImg.key = key;
            strcpy(logBuffer[curLog].oImg.value, oldImg->rec.lRec[r_idx].value);
            strcpy(logBuffer[curLog].nImg.value, value);
            //logBuffer[curLog].PrevLSN = oldImg->pageLSN;
            oldImg->pageLSN = logBuffer[curLog].LSN; 
            break;
        case COMMIT:
            break;
        case ABORT:
            break;
    }
    curLog ++;
    if(curLog == LOGNUM){
        log_flush();
        memset(logBuffer, 0, LOGNUM * sizeof(Log));
        curLog = 0;
    }
}

void db_recovery(){
    int i, size, b_idx, r_idx, endLSN;
    int state = -1;
    lseek(log_fd, 0, SEEK_SET);
    while((size = read(log_fd, logBuffer, LOGNUM * sizeof(Log))) > 0){
        endLSN = size / LOGSIZE;
        // redo pass
        for(i = 0; i < endLSN; i ++){
            if(logBuffer[i].type == UPDATE){
                if(!tables[logBuffer[i].table_id - 1].inUse){
                    char tmp[10] = {0};
                    sprintf(tmp, "DATA%d", logBuffer[i].table_id);
                    open_table(tmp);
                    tables[logBuffer[i].table_id - 1].recovered = 1;
                }
                b_idx = bufferRead(logBuffer[i].PGN * PGSIZE, NULL, logBuffer[i].table_id, 0);
                appointFrame(b_idx);
                if(buffTable[b_idx].frame.pageLSN < logBuffer[i].LSN){
                    r_idx = (logBuffer[i].offset - PGHEADSIZE) / LRECSIZE;
                    buffTable[b_idx].frame.rec.lRec[r_idx].key = logBuffer[i].nImg.key;
                    strcpy(buffTable[b_idx].frame.rec.lRec[r_idx].value, logBuffer[i].nImg.value);
                    buffTable[b_idx].frame.pageLSN = logBuffer[i].LSN;
                    buffTable[b_idx].is_dirty = 1;
                }
                terminateFrame(b_idx);
            }
        }
    }
    flushedLSN = logBuffer[endLSN - 1].LSN;
    //undo pass 
    i = endLSN - 1;
    while(1){
        while(i >=0){
            switch(state){
                case -1:
                    if(logBuffer[i].type == COMMIT) {
                        state = COMMIT;
                    }
                    else if(logBuffer[i].type == ABORT) state = ABORT;
                    else {
                        i++;
                        state = UPDATE;
                    }
                    i--;
                    break;
                case ABORT:
                case COMMIT:
                    while(logBuffer[i].type != BEGIN) {
                        i--;
                        if(i < 0) break;
                    }
                    if(logBuffer[i].type == BEGIN && i >= 0) {
                        state = -1;
                        i--;
                    }
                    break;
                case UPDATE:
                    //undo
                    if(logBuffer[i].type == BEGIN){
                        state = -1;
                        i--;
                    }
                    else{
                        b_idx = bufferRead(logBuffer[i].PGN * PGSIZE, NULL, logBuffer[i].table_id, 0);
                        appointFrame(b_idx);
                        r_idx = (logBuffer[i].offset - PGHEADSIZE) / LRECSIZE;
                        buffTable[b_idx].frame.rec.lRec[r_idx].key = logBuffer[i].oImg.key;
                        strcpy(buffTable[b_idx].frame.rec.lRec[r_idx].value, logBuffer[i].oImg.value);
                        buffTable[b_idx].is_dirty = 1;
                        terminateFrame(b_idx);
                        i--;
                    }
                    break;
            }
        }
        if(lseek(log_fd, -(LOGNUM*sizeof(Log)), SEEK_CUR) == -1) break;
        size = read(log_fd, logBuffer, LOGNUM * sizeof(Log));
        lseek(log_fd, -(LOGNUM* sizeof(Log)), SEEK_CUR);
        size = size / LOGSIZE;
        i = size - 1;
    }
    memset(logBuffer, 0, LOGNUM * sizeof(Log));
    curLog = 0;
}

// db를 초기화시킨다, 여기선 버퍼 테이블과 열려질 파일들의 테이블을 할당하게 된다.
int init_db (int buf_num){
    xInfo.xid = 1;
    xInfo.xOn = 0;
    buffNum = buf_num;
    if((buffTable = (Buffer*)calloc(buffNum, sizeof(Buffer)))==NULL)return -1;
    if((tables = (Table*)calloc(TABLENUM, sizeof(Table)))==NULL)return -1;
    if((logBuffer = (Log*)calloc(LOGNUM, sizeof(Log))) == NULL)return -1;
    if ((log_fd = open("Log", O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0644)) == -1){
        if((log_fd = open("Log", O_RDWR | O_SYNC)) == -1) return -1;
    }
    db_recovery();
    return 0;
}

// db를 종료시킨다, 열려있는 버퍼의 모든 프레임들을 디스크에 써야할 경우 쓰면서 비우고 나서 종료한다.
int shutdown_db(){
    int i;
    for(i = 0 ; i < buffNum; i ++){
        if(currentInBuff == 0) break;
        if(buffTable[i].table_id != 0){
            if(buffTable[i].is_dirty) {
                if(buffTable[i].frame.pageLSN > flushedLSN && curLog != 0) log_flush(); //WAL Policy
                fileWrite(buffTable[i].pageOff, &buffTable[i].frame, tables[buffTable[i].table_id - 1].fd);
            }
            buffTable[i].table_id = 0;
            currentInBuff--;
        }
    }
    for(i = 0 ; i < TABLENUM; i ++){
        if(tables[i].inUse){
            if(tables[i].header.pageLSN > flushedLSN && curLog != 0) log_flush(); // WAL Policy
            fileWrite(0, (page*)&tables[i].header, tables[i].fd);
        }
    }
    free(buffTable);
    free(tables);
    return 0;
} 

int close_table(int table_id){
    int i;
    for(i = 0 ; i < buffNum; i++){
        if(currentInBuff == 0) break;
        if(buffTable[i].table_id == table_id){
            if(buffTable[i].is_dirty) {
                if(buffTable[i].frame.pageLSN > flushedLSN && curLog != 0) log_flush(); // WAL Policy
                fileWrite(buffTable[i].pageOff, &buffTable[i].frame, tables[table_id - 1].fd);
            }
            frame_init(i, 0, 0);
            currentInBuff--;
        }
    }
    if(tables[table_id - 1].header.pageLSN > flushedLSN && curLog != 0) log_flush(); // WAL Policy
    fileWrite(0, (page*)&tables[table_id - 1].header, tables[table_id - 1].fd);
    memset(&tables[table_id - 1], 0, sizeof(Table));
    return 0;
} 

int open_table(char* pathname){
    // 파일이 없으면 파일을 만들고 그 안에 헤더 페이지만 넣어놓는다.
    int i, fd, tableID;
    h_page fileRoot;
    if(tables == NULL || buffTable == NULL) {
        printf("Initialize the buffer first!\n");
        return -1;
    }
    if ((fd = open(pathname, O_RDWR | O_CREAT | O_EXCL | O_SYNC, 0644)) > 0){
        i = atoi(&pathname[4]) - 1;
        if(!tables[i].inUse || !strcmp(tables[i].name, pathname)){
            memset(&tables[i].header, 0, PGSIZE);
            tables[i].header.numPages = 1;
            tables[i].fd = fd;
            tables[i].inUse = 1;
            strcpy(tables[i].name, pathname);
            tableID = i + 1;
        }
        fileWrite(0, (page*)&tables[i].header, tables[i].fd);
    }
    else {
        // 파일이 존재한다면 염
        if((fd = open(pathname, O_RDWR | O_SYNC)) == -1) return -1;
        memset(&fileRoot, 0, PGSIZE);
        lseek(fd, 0, SEEK_SET);
        read(fd, &fileRoot, sizeof(h_page));
        i = atoi(&pathname[4]) - 1;
        if(!tables[i].inUse || !strcmp(tables[i].name, pathname)){
            tables[i].fd = fd;
            tables[i].inUse = 1;
            if(!tables[i].recovered)memcpy(&tables[i].header, &fileRoot, sizeof(h_page));
            tableID = i + 1;
        }
    }
    if(buffPage == NULL) buffPage = (page*)calloc(1, sizeof(page));
    if(neighbor == NULL) neighbor = (page*)calloc(1, sizeof(page));
    gv_init();
    // 입력 파일 디스크립터
    return tableID;
}

char* find(int table_id, int64_t key){
	if(tables[table_id - 1].inUse == 0){
        printf("You should open the file First!\n");
        return NULL;
    }
    int i = 0;
	find_leaf(table_id, key); // 우선 key 가 들어있을 만한 leaf node를 찾는다.
	if (currentOff == 0) return NULL;
	for (i = 0; i < buffPage->numKeys; i++){  // 이제 그 leaf node에서 해당 key에 맞는 데이터가 들어있는 지 찾는다!
		if (buffPage->rec.lRec[i].key == key) {
            retBuff[0] = '\0';
            strcpy(retBuff, buffPage->rec.lRec[i].value);
            terminateFrame(usingIndex);
            gv_init();
            return retBuff;
        }
    }
    terminateFrame(usingIndex);
    gv_init();
    return NULL;
}

int insert(int table_id, int64_t key, char * value){
    if(tables[table_id - 1].inUse == 0){
        printf("You should open the file First!\n");
        return 1;
    }
    int i = 0;
    find_leaf(table_id, key);
	// 이 부분은 만약 이미 키가 트리에 존재한다면 insert를포기하는 부분 -> 중복 값은 허용하지 않음.
    if(currentOff != 0){
        for(i = 0 ; i < buffPage->numKeys; i ++){
            if(buffPage->rec.lRec[i].key == key) {
                terminateFrame(usingIndex);
                gv_init();
                return -1;
            }
        }
    }
	/* Case: the tree does not exist yet.
	 * Start a new tree.
	 */
	if (currentOff == 0){
		start_new_tree(table_id, key, value);
        gv_init();
		return 0;
	}

	// 만약 리프 노드에 넣을 자리가 있다면
	if (buffPage->numKeys < LEAF_ORDER -1) {
        //logging
        //if(xInfo.xOn){
        //  insert log;
        //}
		insert_into_leaf(table_id, key, value);
        terminateFrame(usingIndex);
		gv_init();
		return 0;
	}

	// 만약 리프 노드에 넣을 자리가 없다면 -> splitting 여기서 이제 insert_into_parent 나오고 거기서 다시 splitting, node insertion, new root creation 등 수행.
	insert_into_leaf_after_splitting(table_id, key, value);
	gv_init();
	return 0;

}

int delete(int table_id, int64_t key){
    if(tables[table_id - 1].inUse == 0){
        printf("You should open the file First!\n");
        return -1;
    }
    int i = 0;
    find_leaf(table_id, key);
    if(currentOff == 0) return 0;
    for(i = 0 ; i < buffPage->numKeys; i ++){
        if(buffPage->rec.lRec[i].key == key){
            //logging
            //if(xInfo.xOn){
            //  delete log;
            //}
      		delete_entry(table_id, key); // 삭제한다.
	    	gv_init();
    		return 0;
        }
    }
    return 0;
}

int update(int table_id, int64_t key, char* value){
    if(tables[table_id - 1].inUse == 0){
        printf("You should open the file First!\n");
        return -1;
    }
    int i;
    int startOffset;
    find_leaf(table_id, key);
    if(currentOff == 0) return 0;
    for(i = 0 ; i < buffPage->numKeys; i ++){
        if(buffPage->rec.lRec[i].key == key){
//     		logging first
     		if(xInfo.xOn) {
                startOffset = currentOff + PGHEADSIZE + (LRECSIZE * i);
                log_update(table_id, &buffTable[usingIndex].frame, key, value, startOffset, UPDATE);
            }
            strcpy(buffTable[usingIndex].frame.rec.lRec[i].value, value);
            buffTable[usingIndex].is_dirty = 1;
            terminateFrame(usingIndex);
	    	gv_init();
    		return 0;
        }
    }
    return 0;
}

void log_flush(){
//    printf("=======\nflushing log\ncurLog is : %d\nflushedLSN is %ld\nlast LSN is %ld\nflushing size is %ld\n", curLog, flushedLSN, logBuffer[curLog -1].LSN, logBuffer[curLog - 1].LSN - flushedLSN);
    lseek(log_fd, 0, SEEK_END);
    write(log_fd, logBuffer, sizeof(Log) * curLog);
    flushedLSN = logBuffer[curLog - 1].LSN;
    memset(logBuffer, 0, sizeof(Log) * curLog);
    curLog = 0;
}

// Transaction Functions:
int begin_transaction(){
    xInfo.xOn = 1;
    log_update(0, NULL, 0, NULL, 0, BEGIN);
    return 0;
}

int commit_transaction(){
    log_update(0, NULL, 0, NULL, 0, COMMIT);
    log_flush();
    xInfo.xid ++;
    xInfo.xOn = 0;
    return 0;
}

int abort_transaction(){
    int b_idx, r_idx, undo_fd;
    Log undoLog;
    log_flush();
    memset(&undoLog, 0, LOGSIZE);
    if((undo_fd = open("Log", O_RDWR | O_SYNC)) == -1) return -1;
    lseek(undo_fd, -LOGSIZE, SEEK_END);
    read(undo_fd, &undoLog, LOGSIZE);
    while(undoLog.LSN != xInfo.initLSN){
        if(undoLog.xid == xInfo.xid){
            b_idx = bufferRead(undoLog.PGN * PGSIZE, NULL, undoLog.table_id, 0);
            appointFrame(b_idx);
            r_idx = (undoLog.offset - PGHEADSIZE) / LRECSIZE;
            if(buffTable[b_idx].frame.rec.lRec[r_idx].key == undoLog.oImg.key){
    //            if(undoLog.nImg.key == 1513){
//            printf("in Log : %ld, %s\nin Buffer : %ld, %s\nin Log : pageOff %ld, offset %ld\nand pageLSN : %ld\n", undoLog.nImg.key, undoLog.nImg.value, buffTable[b_idx].frame.rec.lRec[r_idx].key, buffTable[b_idx].frame.rec.lRec[r_idx].value, undoLog.PGN, undoLog.offset, buffTable[b_idx].frame.pageLSN);
      //          printf("curLog %d t_id %d, oImg key %ld, oImg val %s, pageOff %ld\n and buffer: %ld, %s\n", curLog, undoLog.table_id, undoLog.oImg.key, undoLog.oImg.value, undoLog.PGN + undoLog.offset, buffTable[b_idx].frame.rec.lRec[r_idx].key, buffTable[b_idx].frame.rec.lRec[r_idx].value);
        //    }
            log_update(undoLog.table_id, &(buffTable[b_idx].frame), undoLog.oImg.key, undoLog.oImg.value, undoLog.PGN * PGSIZE + undoLog.offset, UPDATE);
          //      if(undoLog.nImg.key == 1513){
           // printf("in Log : %ld, %s old : %ld, %s\nin Buffer : %ld, %s\nin Log : pageOff %ld, offset %ld\nand pageLSN : %ld\n", logBuffer[curLog - 1].nImg.key, logBuffer[curLog - 1].nImg.value, logBuffer[curLog - 1].oImg.key, logBuffer[curLog - 1].oImg.value, buffTable[b_idx].frame.rec.lRec[r_idx].key, buffTable[b_idx].frame.rec.lRec[r_idx].value, logBuffer[curLog - 1].PGN, logBuffer[curLog - 1].offset, buffTable[b_idx].frame.pageLSN);
            //}
            buffTable[b_idx].frame.rec.lRec[r_idx].key = undoLog.oImg.key;
            strcpy(buffTable[b_idx].frame.rec.lRec[r_idx].value, undoLog.oImg.value);
            buffTable[b_idx].is_dirty = 1;
            }
            terminateFrame(b_idx);
        }
        memset(&undoLog, 0, LOGSIZE);
        lseek(undo_fd, -(2 * LOGSIZE), SEEK_CUR);
        read(undo_fd, &undoLog, LOGSIZE);
    }
    log_update(0, NULL, 0, NULL, 0, ABORT);
   // printf("curLog : %d\n", curLog);
    log_flush();
    xInfo.xid ++;
    xInfo.xOn = 0;
    close(undo_fd);
    return 0;
}

// Internal Functions::
// 검색함수들

// Find_leaf함수 :: root에서 시작해 leaf 까지 탐색하며 내려가는 함수, key를 토대로 찾게 되며 원하는 key를 가지고 있는 (반드시 해당 데이터를 들고 있는 건 아니다) leaf node를 반환한다
void find_leaf(int table_id, int64_t key) {
	int i = 0;
    int idx = 0;
	ulint temp = 0;
	currentOff = tables[table_id - 1].header.rootPageOff; // 루트부터 검색 시작
	if (currentOff == 0) { // 루트가 0이면 빈 나무인것
		return;
	}
    idx = bufferRead(currentOff, buffPage, table_id, 1);
    appointFrame(idx);
	while (!buffPage->isLeaf) {	// 인터널 페이지들을 탐색하며 내려간다.
		i = 0;
		if (key < buffPage->rec.iRec[0].key)temp = buffPage->extraOff;	// 만약 첫번째 키보다 작다면 extraoff에 있다.
		else{
			while (i < buffPage->numKeys) {	// 아닐 경우 키들을 탐색하며 알맞은 위치를 찾는다.
				if (key >= buffPage->rec.iRec[i].key) i++;
				else break;
			}
			temp = buffPage->rec.iRec[i-1].pageOff; // 찾게되면 해당 페이지로 넘어가기 위해 갱신
		}
        terminateFrame(idx);
		currentOff = temp;
		idx = bufferRead(currentOff, buffPage, table_id, 1);
        appointFrame(idx);
	}
    usingIndex = idx;
}

// cut 함수 :: 노드가 너무 클경우 둘로 짜를만한 적당한 자리를 찾는 함수
int cut( int length ) {
	if (length % 2 == 0)
		return length/2;
	else
		return length/2 + 1;
}

// INSERTION

// get_left_index :: parent에서 삽입할 key의 왼쪽 노드의 pointer의 index를 찾는 함수
int get_left_index(ulint curOff) {

	int left_index = 0;
	if(buffPage->extraOff == curOff) return -1;
	while (left_index < buffPage->numKeys && 
			buffPage->rec.iRec[left_index].pageOff != curOff) // 현 노드가 아닌 동안 parent의 포인터들을 왼쪽부터 차례로 확인한다.
		left_index++;
	return left_index;
}

// insert_into_leaf :: leaf node에서 key 값에 따라 삽입하는 함수!
void insert_into_leaf(int table_id, int64_t key, char* value) {
	int i, insertion_point;

	insertion_point = 0;
	//넣어야 하는 key가 더 작을 때까지 키값들을 확인한다.
	while (insertion_point < buffPage->numKeys && buffPage->rec.lRec[insertion_point].key < key) insertion_point++;
	// num_keys가 order - 1 보다 작을 때만 insert_into_leaf함수를 호출한다, 따라서 무조건 key[num_keys]는 자리가 있게 된다.
	for (i = buffPage->numKeys; i > insertion_point; i--) {
		buffPage->rec.lRec[i].key = buffPage->rec.lRec[i-1].key;							// 찾은 위치 뒤 값들을 다 옮김
		strcpy(buffPage->rec.lRec[i].value, buffPage->rec.lRec[i-1].value);
	}
	buffPage->rec.lRec[insertion_point].key = key;	// 이제 삽입해야하는 실제 키와 데이터를 집어 넣는다.
	strcpy(buffPage->rec.lRec[insertion_point].value, value);
	buffPage->numKeys++;
	bufferWrite(currentOff, buffPage, table_id);
}


// insert_into_leaf_after_splitting :: 꽉 차 있는 노드에 삽입할 경우 먼저 스플릿하는데
// 스플릿 하고나서 삽입하는 함수
void insert_into_leaf_after_splitting(int table_id, int64_t key, char* value) {
	int insertion_index, split, new_key, i, j;
    int f_idx = -1;
    memset(neighbor, 0, PGSIZE);
    neighbor->isLeaf = 1;
	if(tables[table_id - 1].header.freePageOff == 0){
		addedOff = tables[table_id - 1].header.numPages * PGSIZE;
		tables[table_id - 1].header.numPages++;
	}
	else {
		ulint freeOff;
        page tmp;
        f_idx = bufferRead(tables[table_id - 1].header.freePageOff, &tmp, table_id, 1);
        appointFrame(f_idx);
        freeOff = tmp.pageOff;
		addedOff = tables[table_id - 1].header.freePageOff;
		tables[table_id - 1].header.freePageOff = freeOff;
	}
	insertion_index = 0;
	// 해당 위치를 찾는다
	while (insertion_index < buffPage->numKeys && buffPage->rec.lRec[insertion_index].key < key)
		insertion_index++;
    
	split = cut(LEAF_ORDER - 1);

	//  위치를 찾은 후에는 일단 만들어둔 임시 배열에다가 키와 포인터를 싹 다 옮기게 된다, 그와 중에 만약 j가 넣어야 되는 위치의 인덱스가 되면 j를 하나 더 늘려서자리를 비워두고  저장을 한다.
    if(insertion_index < split){
        for(i = split - 1, j = 0; i < LEAF_ORDER - 1 ; i ++, j ++){
            neighbor->rec.lRec[j].key = buffPage->rec.lRec[i].key;
            strcpy(neighbor->rec.lRec[j].value, buffPage->rec.lRec[i].value);
            neighbor->numKeys++;
            buffPage->numKeys--;
        }
        for(i = split - 2; i >= insertion_index; i --){
            buffPage->rec.lRec[i+1].key = buffPage->rec.lRec[i].key;
            strcpy(buffPage->rec.lRec[i+1].value, buffPage->rec.lRec[i].value);
        }
        buffPage->rec.lRec[insertion_index].key = key;
        strcpy(buffPage->rec.lRec[insertion_index].value, value);
        buffPage->numKeys++;
    }

    else {
        for(i = split, j = 0 ; i < LEAF_ORDER - 1  ; i ++, j++){
            if(i == insertion_index){
                j++;
            }
            neighbor->rec.lRec[j].key = buffPage->rec.lRec[i].key;
            strcpy(neighbor->rec.lRec[j].value, buffPage->rec.lRec[i].value);
            neighbor->numKeys++;
            buffPage->numKeys--;
        }
        neighbor->rec.lRec[insertion_index - split].key = key;
        strcpy(neighbor->rec.lRec[insertion_index - split].value, value);
        neighbor->numKeys++;
    }

	// 그 후 다음 노드를 참조하는 pointer를 바꿔준다.
	neighbor->extraOff = buffPage->extraOff;
	buffPage->extraOff = addedOff;

    for(i = buffPage->numKeys; i < LEAF_ORDER - 1; i ++){
        buffPage->rec.lRec[i].key = 0;
        memset(buffPage->rec.lRec[i].value, 0, 120);
    }

    for(i = neighbor->numKeys; i < LEAF_ORDER - 1; i ++){
        neighbor->rec.lRec[i].key = 0;
        memset(neighbor->rec.lRec[i].value, 0, 120);
    }

	// 그리고 부모를 바꿔주고, 부모에 하나의 노드를 추가했다는 걸 알려주며 새로운 키값을 하나 주어 부모에 추가시킨다.
	neighbor->pageOff = buffPage->pageOff;
	new_key = neighbor->rec.lRec[0].key;
	// 실제 저장하는 부분
	bufferWrite(addedOff, neighbor, table_id);
    if(f_idx != -1) terminateFrame(f_idx);
	bufferWrite(currentOff, buffPage, table_id);
    terminateFrame(usingIndex);
	// 새로운 키를 이제 부모에게 추가한다.
	insert_into_parent(table_id, new_key);
}


//insert_into_node :: 여태 했던 것이 leaf node에서 추가했을 경우면, 이제 거기서 올라온 키값을 parent (internal node)에 추가해주는 함수다.
void insert_into_node(int table_id, int left_index, int64_t key) {
	int i;
	// 우선 삽입위치 오른쪽 것들을 다 우측으로 한칸 이동시킨다.
	for (i = buffPage->numKeys; i > left_index + 1; i--) {
		buffPage->rec.iRec[i].pageOff = buffPage->rec.iRec[i-1].pageOff;
		buffPage->rec.iRec[i].key = buffPage->rec.iRec[i-1].key;
	}

	// 그리고 나서 left_index의 + 1 자리에 (left_index는 현재 left leaf node를 참조 중이기 때문에) right를 넣어주고, key 의 [left_index] (internal은 항상 pointer가 index가 key 보다 1 크니까 ) 해당 키를 넣어준다. 그리고 num_key 증가헀으니 하나 늘려준다.
	buffPage->rec.iRec[left_index+1].pageOff = addedOff;
	buffPage->rec.iRec[left_index+1].key = key;
	buffPage->numKeys++;
	bufferWrite(currentOff, buffPage, table_id);
}


// insert_into_node_after_splitting :: 아까 parent에 삽입하려는데 만약 이미 꽉 차 있다면 parent마저 split 하고 삽입한다, 따라서 마지막엔 역시 insert_into_parent가 다시 호출된다.
void insert_into_node_after_splitting(int table_id, int left_index, int64_t key) {
	int i, j, split, k_prime;
    int f_idx = -1;
	memset(neighbor, 0, PGSIZE);
    neighbor->isLeaf = 0;
    page tmp;
	//int64_t  temp_keys[INTERNAL_ORDER];
	//ulint temp_pageOffs[INTERNAL_ORDER];
	ulint rightPageOff = addedOff;
	// 역시 새로운 페이지의 할당 부분, 프리리스트를 쓸 수도 있다.
	if(tables[table_id - 1].header.freePageOff == 0){
		addedOff = tables[table_id - 1].header.numPages * PGSIZE;
		tables[table_id - 1].header.numPages++;
	}
	else {
		ulint freeOff;
        f_idx = bufferRead(tables[table_id - 1].header.freePageOff, &tmp, table_id, 1);
        appointFrame(f_idx);
        freeOff = tmp.pageOff;
		addedOff = tables[table_id - 1].header.freePageOff;
		tables[table_id - 1].header.freePageOff = freeOff;
	}
	split = cut(INTERNAL_ORDER);

    if(left_index < (split-2)){
        for(i = split - 1, j = 0; i < INTERNAL_ORDER - 1 ; i ++, j ++){
            neighbor->rec.iRec[j].key = buffPage->rec.iRec[i].key;
            neighbor->rec.iRec[j].pageOff = buffPage->rec.iRec[i].pageOff;
            neighbor->numKeys++;
            buffPage->numKeys--;
        }
        k_prime = buffPage->rec.iRec[split-2].key;
        neighbor->extraOff = buffPage->rec.iRec[split-2].pageOff;
        for(i = split - 3; i >= left_index + 1; i --){
            buffPage->rec.iRec[i+1].key = buffPage->rec.iRec[i].key;
            buffPage->rec.iRec[i+1].pageOff = buffPage->rec.iRec[i].pageOff;
        }
        buffPage->rec.iRec[left_index+1].key = key;
        buffPage->rec.iRec[left_index+1].pageOff = rightPageOff;
    }
    else if(left_index == (split - 2)){
        k_prime = key;
        neighbor->extraOff = rightPageOff;
        for(i = split - 1, j = 0; i < INTERNAL_ORDER - 1; i++, j++){
            neighbor->rec.iRec[j].key = buffPage->rec.iRec[i].key;
            neighbor->rec.iRec[j].pageOff = buffPage->rec.iRec[i].pageOff;
            neighbor->numKeys++;
            buffPage->numKeys--;
        }
    }
    else {
        k_prime = buffPage->rec.iRec[split - 1].key;
        neighbor->extraOff = buffPage->rec.iRec[split - 1].pageOff;
        buffPage->numKeys--;
        for(i = split, j = 0 ; i < INTERNAL_ORDER - 1  ; i ++, j++){
            if(i == left_index + 1){
                j++;
            }
            neighbor->rec.iRec[j].key = buffPage->rec.iRec[i].key;
            neighbor->rec.iRec[j].pageOff = buffPage->rec.iRec[i].pageOff;
            neighbor->numKeys++;
            buffPage->numKeys--;
        }
        neighbor->rec.iRec[left_index + 1 - split].key = key;
        neighbor->rec.iRec[left_index + 1 - split].pageOff = rightPageOff;
        neighbor->numKeys++;
    }
	// 본인의 부모를 기존 노드의 부모와 같게 만들어 놓고
	neighbor->pageOff = buffPage->pageOff;
	//이 for문을 돌면서 밑의 포인터에서 옛 parent를 참조하고 있는 노드들을 신 노드로 갱신해준다.
    int idx = bufferRead(neighbor->extraOff, &tmp, table_id, 1);
    appointFrame(idx);
    tmp.pageOff = addedOff;
	bufferWrite(neighbor->extraOff, &tmp, table_id);
    terminateFrame(idx);
	for (i = 0; i < neighbor->numKeys; i++) {
		idx = bufferRead(neighbor->rec.iRec[i].pageOff, &tmp, table_id, 1);
        appointFrame(idx);
        tmp.pageOff = addedOff;
        bufferWrite(neighbor->rec.iRec[i].pageOff, &tmp, table_id);		
        terminateFrame(idx);
	}

    for (i = buffPage->numKeys; i < INTERNAL_ORDER -1; i++){
        buffPage->rec.iRec[i].key = 0;
        buffPage->rec.iRec[i].pageOff = 0;
    }

    for (i = neighbor->numKeys; i < INTERNAL_ORDER - 1; i++){
        neighbor->rec.iRec[i].key = 0;
        neighbor->rec.iRec[i].pageOff = 0;
    }
	bufferWrite(addedOff, neighbor, table_id);
    if(f_idx != 0) terminateFrame(f_idx);
	bufferWrite(currentOff, buffPage, table_id);
    terminateFrame(usingIndex);
	// 이제 다시 parent에 넣는다
	insert_into_parent(table_id, k_prime);
}


// insert_into_parent :: 새로운 걸 insert 했다고 하면, 항상 split이 되게 되고 키값 하나가 parent에게 올라올텐데 그 키 값을 적절한 위치에 넣는 함수다.
void insert_into_parent(int table_id, int64_t key) {
	ulint forLeftIndex = currentOff;
	currentOff = buffPage->pageOff;
	// 만약 기존에 스플릿 하기 전 노드가 루트였다면, 새로운 루트가 탄생하는 것이다
	if (currentOff == 0){
		insert_into_new_root(table_id, forLeftIndex, key);
		return;
	}
	
	usingIndex = bufferRead(currentOff, buffPage, table_id, 1);
    appointFrame(usingIndex);
	// 아닐경우 일단 parent에서 어느 포인터가 left를 가리키고 있는 건지 그 index를 찾는다.
	int left_index = get_left_index(forLeftIndex);
	// 그 후 만약 parent에 새로운 키값을 넣을 자리가 있다면! 그냥 삽입하게 된다.
	if (buffPage->numKeys < INTERNAL_ORDER -1){
		insert_into_node(table_id, left_index, key);
        terminateFrame(usingIndex);
		return;
	}		

	// 하지만 만약 parent도 이미 꽉차 있다면..? parent 역시 스플릿 해야된다
	insert_into_node_after_splitting(table_id, left_index, key);
}


// insert_into_new_root :: insert_into_node_after_splitting에서 이어지는 데, 올라온 값을 새로운 루트로 만들어서 이어준다.
void insert_into_new_root(int table_id, ulint leftOff, int64_t key) {
	page tmp;
    int f_idx = -1;
    memset(buffPage, 0, sizeof(page));
    buffPage->isLeaf = 0;    
	buffPage->rec.iRec[0].key = key;
	buffPage->rec.iRec[0].pageOff = addedOff;
	buffPage->extraOff = leftOff;
	buffPage->numKeys++;
	buffPage->pageOff = 0;
	// 새로운 페이지를 할당할 때는 항상 free list를 검사하며 할당을 한다!
	if(tables[table_id - 1].header.freePageOff == 0){
		tables[table_id - 1].header.rootPageOff = tables[table_id - 1].header.numPages * PGSIZE;
		currentOff = tables[table_id - 1].header.rootPageOff;
		tables[table_id - 1].header.numPages++;
	}
	else{
		ulint freeOff;
		tables[table_id - 1].header.rootPageOff = tables[table_id - 1].header.freePageOff;
		currentOff = tables[table_id - 1].header.rootPageOff;
        f_idx = bufferRead(tables[table_id - 1].header.freePageOff, &tmp, table_id, 1);
        appointFrame(f_idx);
        freeOff = tmp.pageOff;
		tables[table_id - 1].header.freePageOff = freeOff;
	}
	// 이제 전 노드들의 부모를 새로운 루트의 offset으로 바꿔준다.
    bufferWrite(currentOff, buffPage, table_id);
    if(f_idx != 0)terminateFrame(f_idx);
	int idx = bufferRead(leftOff, &tmp, table_id, 1);
    appointFrame(idx);
    tmp.pageOff = currentOff;
    bufferWrite(leftOff, &tmp, table_id);
    terminateFrame(idx);
    neighbor->pageOff = currentOff;
	bufferWrite(addedOff, neighbor, table_id);
}


// start_new_tree :: 첫 시작! 완전 최초 루트 만드는 것이다.
void start_new_tree(int table_id, int64_t key, char * value) {
	int f_idx = -1;
    memset(buffPage, 0, sizeof(page));	
	buffPage->isLeaf = 1;
    buffPage->rec.lRec[0].key = key;
	strcpy(buffPage->rec.lRec[0].value, value);
	buffPage->extraOff = 0;
	buffPage->pageOff = 0;
	buffPage->numKeys++;
	if(tables[table_id - 1].header.freePageOff == 0){
		tables[table_id - 1].header.rootPageOff = tables[table_id - 1].header.numPages * PGSIZE;
		currentOff = tables[table_id - 1].header.rootPageOff;
		tables[table_id - 1].header.numPages++;
	}
	else{
		ulint freeOff;
        page tmp;
		tables[table_id - 1].header.rootPageOff = tables[table_id - 1].header.freePageOff;
		currentOff = tables[table_id - 1].header.rootPageOff;
            
        f_idx = bufferRead(tables[table_id - 1].header.freePageOff, &tmp, table_id, 1);
        appointFrame(f_idx);
        freeOff = tmp.pageOff;
		tables[table_id - 1].header.freePageOff = freeOff;
	}
    bufferWrite(currentOff, buffPage, table_id);
    if(f_idx != 0)terminateFrame(f_idx);
}

// DELETION.

// remove_entry_from_node :: 노드에 존재하는 정확한 값을 인자로 받아서 그 걸 노드에서 지워버림! 리프든 인터널이든 상관이없다, delete_entry랑 다른점은, 상하관계라는것이다 이 함수가 delete_entry 에 포함된다
void remove_entry_from_node(int table_id, int64_t key) {

	int i = 0;
    int key_idx = 0;
	// 키 배열에 그 키를 찾아서 그 다음 키들을 다 앞으로 한칸씩 땡긴다.
	//리프일 때와 인터널일 때 각각 다르게 땡기게 된다.
	if(buffPage->isLeaf){
		for(i = 0; i < buffPage->numKeys; i++){
            if(buffPage->rec.lRec[i].key == key){
                key_idx = i;
                break;
            }
        }
        if (i == buffPage->numKeys){
            printf("No such key!: %ld\n", key);
            return;
        }
		for (i = key_idx; i < buffPage->numKeys - 1; i++){
			buffPage->rec.lRec[i].key = buffPage->rec.lRec[i + 1].key;
			strcpy(buffPage->rec.lRec[i].value, buffPage->rec.lRec[i + 1].value);
		}
        for(i = buffPage->numKeys - 1; i < LEAF_ORDER - 1; i ++){
    		buffPage->rec.lRec[i].key = 0;
		    memset(buffPage->rec.lRec[i].value, 0, 120);
        }
        buffPage->numKeys --;
	}
	else{
		for(i = 0 ; i < buffPage->numKeys; i ++){
            if(buffPage->rec.iRec[i].key == key){
                key_idx = i;
                break;
            }
        }
        if (i == buffPage->numKeys) {
            printf("No such key!: %ld\n", key);
            return;
        }
		for (i = key_idx; i < buffPage->numKeys - 1; i++){
			buffPage->rec.iRec[i].key = buffPage->rec.iRec[i + 1].key;
			buffPage->rec.iRec[i].pageOff = buffPage->rec.iRec[i + 1].pageOff;
		}
        for(i = buffPage->numKeys-1; i < INTERNAL_ORDER - 1; i ++){
		    buffPage->rec.iRec[i].key = 0;
		    buffPage->rec.iRec[i].pageOff = 0;
        }
        buffPage->numKeys --;
	}
	// 그 후 저장을한다.
	bufferWrite(currentOff, buffPage, table_id);
}

// adjust_root :: 모든 딜리트가 끝나고, 합병 재배치 등 다 끝나고 루트의 상태를 살핀다, 만약 루트가 비어있는 (key가 없다) 상태라면 루트를 새로운 루트로 바꿔주는 함수이다
void adjust_root(int table_id) {
	// 루트가 key가 있으면 루트는 아직 자격이 있다, 그냥 루트로 남아있는다.
	if (buffPage->numKeys > 0){
		return;
	}

	// 이제 루트가 비었을 경우다.

	// 루트가 인터널 노드라면 -> 그말은 자식이 있다는 거다, 하지만 key는 없기 때문에 맨 앞 pointer[0](extraOff)만 자식으로 가지고 있는 거다, 그래서 pointer[0]을 새로운 루트로 바꿔버리는 것이다.
	if (!buffPage->isLeaf) {
        page tmp;
		tables[table_id - 1].header.rootPageOff = buffPage->extraOff;
        int idx = bufferRead(tables[table_id - 1].header.rootPageOff, &tmp, table_id, 1);
        appointFrame(idx);
        tmp.pageOff = 0;
		bufferWrite(tables[table_id - 1].header.rootPageOff, &tmp, table_id);
        terminateFrame(idx);
	}

	// 만약 루트가 리프 노드라면 -> 자식도 없는데 본인은 key도 없다, 그러면 나무 전체가 빈거다, 따라서 그냥 나무 자체를 없다고 한다.
	else {
		tables[table_id - 1].header.rootPageOff = 0;
	}
	// 나무의 옛 루트를 초기화 시키고 프리리스트에 추가한다.
	memset(buffPage, 0, PGSIZE);
	if(tables[table_id - 1].header.freePageOff != 0) buffPage->pageOff = tables[table_id - 1].header.freePageOff;
	tables[table_id - 1].header.freePageOff = currentOff;
	bufferWrite(currentOff, buffPage, table_id);
}

// coalesce_nodes :: 지웠더니 너무 사이즈가 order / 2 보다 작을 때면 옆의 sibling 노드를 보고 합쳐도 될 사이즈면 둘이 합친다.
void coalesce_nodes(int table_id, int neighbor_index, int64_t k_prime, int n_idx, page* parent, int p_idx) {

	int i, j, neighbor_insertion_index, n_end;
	page * tmp;
	ulint freeOff = 0;
	ulint tmpOff;

	// 만약 neighbor이 없다 = 제일 왼쪽 노드다 = neighbor_index == -2 -> 그러면 오른쪽 노드랑 바꿔준다. 이 함수를 호출한 함수에서 neighbor에다가 position[0]에 있는 노드를 담아 줬는데 얘가 바로 오른쪽 노드일테니 (본인은 extraOff이니까) 서로 그냥 자리를 바꾼다.

	if (neighbor_index == -2) {
		tmp = buffPage;
		buffPage = neighbor;
		neighbor = tmp;
		tmpOff = currentOff;
		currentOff = remainingOff;
		remainingOff = tmpOff;
	}

	// 이제 neighbour의 끝에다가 붙이기 위하여 neighbor_index 저기다가 neighbor->num_keys를 넣는다
	neighbor_insertion_index = neighbor->numKeys;
	//printf("neighbor_index : %d, neighbor_insertion_index : %d\n", neighbor_index, neighbor_insertion_index);
	// n이 만약 인터널 노드라면 우선 k_prime (부모가 들고 있는 해당 키와 위치) 랑 이웃노드를 다 한 노드에 붙힌다.

	if (!buffPage->isLeaf) {

		// Append k_prime.
		neighbor->rec.iRec[neighbor_insertion_index].key = k_prime;
		neighbor->rec.iRec[neighbor_insertion_index].pageOff = buffPage->extraOff;
		neighbor->numKeys++;

        n_end = buffPage->numKeys;
		// 이웃 노드를 붙힌다
		for (i = neighbor_insertion_index + 1, j = 0; j < n_end; i++, j++) {
			neighbor->rec.iRec[i].key = buffPage->rec.iRec[j].key;
			neighbor->rec.iRec[i].pageOff = buffPage->rec.iRec[j].pageOff;
            buffPage->numKeys--;
            neighbor->numKeys++;
		}

        page temp;
        int idx;
		// 모든 자식들을 하나의 부모만 가리키게끔 한다.
		for (i = neighbor_insertion_index; i < neighbor->numKeys; i++) {
            idx = bufferRead(neighbor->rec.iRec[i].pageOff, &temp, table_id, 1);
            appointFrame(idx);
            temp.pageOff = remainingOff;
			bufferWrite(neighbor->rec.iRec[i].pageOff, &temp, table_id);
            terminateFrame(idx);
		}
	}

	// In a leaf, append the keys and pointers of n to the neighbor.Set the neighbor's last pointer to point to what had been n's right neighbor.

	else {
        n_end = buffPage->numKeys;
		for (i = neighbor_insertion_index, j = 0; j < n_end; i++, j++) {
			neighbor->rec.lRec[i].key = buffPage->rec.lRec[j].key;
			strcpy(neighbor->rec.lRec[i].value, buffPage->rec.lRec[j].value);
			neighbor->numKeys++;
            buffPage->numKeys--;
		}
		neighbor->extraOff = buffPage->extraOff;
	}
	// 이제 이웃 노드를 저장하고
	bufferWrite(remainingOff, neighbor, table_id);
    terminateFrame(n_idx);
    memset(buffPage, 0, PGSIZE);
	// 현 노드는 삭제해야되기 때문에 프리리스트에 추가한다.
	if(tables[table_id - 1].header.freePageOff != 0)freeOff = tables[table_id - 1].header.freePageOff;
	tables[table_id - 1].header.freePageOff = currentOff;
	buffPage->pageOff = freeOff;
	bufferWrite(currentOff, buffPage, table_id);
    terminateFrame(usingIndex);
	// 현 노드를 전부 삭제하고.
	currentOff = neighbor->pageOff;
    memcpy(buffPage, parent, PGSIZE);
    usingIndex = p_idx;
	// 부모 노드를 가리키게끔 바꿔준다.
	delete_entry(table_id, k_prime);// 그리고 부모에서 해당 키를 삭제
}

// 키의 재배치 하는 함수이다.
void redistribute_nodes(int table_id, int neighbor_index, int k_prime_index, int64_t k_prime, page * parent) {  

	int i, idx;
	page tmp;
	if (neighbor_index != -2) { // 만약 현 노드가 extraOff에 있는 게 아닌 경우, neighbor 바로 왼쪽
		if (!buffPage->isLeaf){	// 만약 현 노드가 인터널노드라면
			for(i = buffPage->numKeys; i > 0 ; i--){// 값들을 전부 오른쪽으로 옮긴다.
				buffPage->rec.iRec[i].key = buffPage->rec.iRec[i - 1].key;
				buffPage->rec.iRec[i].pageOff = buffPage->rec.iRec[i - 1].pageOff;
			}
			buffPage->rec.iRec[0].pageOff = buffPage->extraOff;
            buffPage->rec.iRec[0].key = k_prime;
			// extraOff을 추가한다.
			buffPage->extraOff = neighbor->rec.iRec[neighbor->numKeys - 1].pageOff;
			neighbor->rec.iRec[neighbor->numKeys - 1].pageOff = 0;

			parent->rec.iRec[k_prime_index].key = neighbor->rec.iRec[neighbor->numKeys - 1].key;
			bufferWrite(buffPage->pageOff, parent, table_id);
			neighbor->rec.iRec[neighbor->numKeys - 1].key = 0;

            idx = bufferRead(buffPage->extraOff, &tmp, table_id, 1);
            appointFrame(idx);
            tmp.pageOff = currentOff;
			bufferWrite(buffPage->extraOff, &tmp, table_id);
            terminateFrame(idx);
			// 그 후 저장
		}
		else{	// 리프인경우에도  그냥 값을 전부 오른쪽으로 옮긴디 이웃노드에서 현노드로 하나 옮기고 저장.
			for(i = buffPage->numKeys; i > 0; i--){
				buffPage->rec.lRec[i].key = buffPage->rec.lRec[i - 1].key;
				strcpy(buffPage->rec.lRec[i].value, buffPage->rec.lRec[i - 1].value);
			}

			strcpy(buffPage->rec.lRec[0].value, neighbor->rec.lRec[neighbor->numKeys - 1].value);
			memset(neighbor->rec.lRec[neighbor->numKeys - 1].value, 0, 120);

			buffPage->rec.lRec[0].key = neighbor->rec.lRec[neighbor->numKeys - 1].key;
			neighbor->rec.lRec[neighbor->numKeys - 1].key = 0;
			
			parent->rec.iRec[k_prime_index].key = buffPage->rec.lRec[0].key;
			bufferWrite(buffPage->pageOff, parent, table_id);
		}
	}
	else {  // 다음의 경우는 extraOff일 때인데 이 때 하는 일은 이웃노드의 첫째 값을 빼서 현 노드에 넣고 이웃노드를 다 왼쪽으로 땡긴다.
		if (buffPage->isLeaf) {
			buffPage->rec.lRec[buffPage->numKeys].key = neighbor->rec.lRec[0].key;
			strcpy(buffPage->rec.lRec[buffPage->numKeys].value, neighbor->rec.lRec[0].value);
			
            for (i = 0; i < neighbor->numKeys - 1; i++) {
				neighbor->rec.lRec[i].key = neighbor->rec.lRec[i + 1].key;
				strcpy(neighbor->rec.lRec[i].value, neighbor->rec.lRec[i + 1].value);
			}
		
			parent->rec.iRec[k_prime_index].key = neighbor->rec.lRec[0].key;
			bufferWrite(buffPage->pageOff, parent, table_id);

			// 마지막 껀 일단 초기화
			neighbor->rec.lRec[neighbor->numKeys - 1].key = 0;
			memset(neighbor->rec.lRec[neighbor->numKeys - 1].value, 0 , 120);
		}
		else {
			buffPage->rec.iRec[buffPage->numKeys].key = k_prime;
			buffPage->rec.iRec[buffPage->numKeys].pageOff = neighbor->extraOff;
			neighbor->extraOff = neighbor->rec.iRec[0].pageOff;

            idx = bufferRead(buffPage->rec.iRec[buffPage->numKeys].pageOff, &tmp, table_id, 1);
            appointFrame(idx);
            tmp.pageOff = currentOff;
			bufferWrite(buffPage->rec.iRec[buffPage->numKeys].pageOff, &tmp, table_id);
            terminateFrame(idx);

			parent->rec.iRec[k_prime_index].key = neighbor->rec.iRec[0].key;
			bufferWrite(buffPage->pageOff, parent, table_id);
            
			for (i = 0; i < neighbor->numKeys - 1; i++) {
				neighbor->rec.iRec[i].key = neighbor->rec.iRec[i + 1].key;
				neighbor->rec.iRec[i].pageOff = neighbor->rec.iRec[i + 1].pageOff;
			}

			neighbor->rec.iRec[neighbor->numKeys - 1].key = 0;
			neighbor->rec.iRec[neighbor->numKeys - 1].pageOff = 0;
		}
	}
	// 이제 전부 저장한다.
	buffPage->numKeys++;
	neighbor->numKeys--;
	bufferWrite(currentOff, buffPage, table_id);	
    bufferWrite(remainingOff, neighbor, table_id);
}


//delete_entry :: 어떤 특정한 값을 노드에서 지우는 것! 레코드, 키 등 다 리프에서 지우고 나서 비트리에 맞게끔 다 맞춰줌 ! -> 병합, 재배치 등등 여기서 해줌

void delete_entry(int table_id, int64_t key) {

	int min_keys, i;
	int neighbor_index;
	int k_prime_index;
	int64_t	k_prime;
	int capacity;
	bool existing = false;
    page parent;
    memset(neighbor, 0, PGSIZE);
    remainingOff = 0;
	// 일단 노드에서 해당 키와 포인터를 지운다.
	remove_entry_from_node(table_id, key);
	//printf("currentOff ! : %ld\n", currentOff);
	// 만약 n 이 루트라면 adjust_root를 호출함.
	if (currentOff == tables[table_id - 1].header.rootPageOff) {
		adjust_root(table_id);
        terminateFrame(usingIndex);
		return;
	}

	// 여기는 딜리트 한 노드를 핸들 해야할 지 말아야 할지 기준을 정하는 것, min_keys보다 작으면 프로세스가 시작되는 것이다. 만약 리프 노드라면, 2, 3으로 쪼개기에 cut(order - 1) 아니라면 cut(order) - 1
	min_keys = buffPage->isLeaf ? cut(LEAF_ORDER - 1) : cut(INTERNAL_ORDER) - 1;

	// min_keys보다 남은 키 수가 많을 경우, 그냥 끝남.
	if (buffPage->numKeys >= min_keys){
        terminateFrame(usingIndex);   
		return;
    }

	// 만약 더 적다면 이제, 합병이나 재배치를 하게 된다.
	int p_idx = bufferRead(buffPage->pageOff, &parent, table_id, 1);
    appointFrame(p_idx);
	// 우선 적절한 합병할 만한 이웃 노드를 찾는다, 그 후에 그 이웃 노드와 본인 노드 사이에 있는 키를 찾는다.
	if(parent.extraOff == currentOff)  neighbor_index = -2;
	else{
	for (i = 0; i < parent.numKeys; i++){
		if (parent.rec.iRec[i].pageOff == currentOff){
			neighbor_index = i - 1;
			existing = true;
		}
	}
	if(!existing){
		printf("Search for nonexistent pointer to node in parent.");
		printf("On: %ld\n", currentOff);
		return;
        //exit(EXIT_FAILURE);
	}
	existing = false;
	}
	// k_prime이 이웃과 본인 사이의 키다.
	k_prime_index = neighbor_index == -2 ? 0 : (neighbor_index + 1);
	k_prime = parent.rec.iRec[k_prime_index].key;
	if(neighbor_index == -1) {
		remainingOff = parent.extraOff;
	}
	else {
		remainingOff = ((neighbor_index == -2) ? (parent.rec.iRec[0].pageOff) : (parent.rec.iRec[neighbor_index].pageOff));
	}
    int n_idx = bufferRead(remainingOff, neighbor, table_id, 1);
    appointFrame(n_idx);

	capacity = buffPage->isLeaf ? LEAF_ORDER : (INTERNAL_ORDER - 1);		

	// Coalescence.
	// 만약 capacity보다 작으면 병합!
	if (neighbor->numKeys + buffPage->numKeys < capacity){
		coalesce_nodes(table_id, neighbor_index, k_prime, n_idx, &parent, p_idx);
		return;
	}

	// Redistribution.

	else {
		redistribute_nodes(table_id, neighbor_index, k_prime_index, k_prime, &parent);
        terminateFrame(usingIndex);
        terminateFrame(n_idx);
        terminateFrame(p_idx);
		return;
	}
}
