#!/usr/bin/python3

'''
    test program
'''

from subprocess import Popen, PIPE
import os
from random import shuffle
from timeit import default_timer as timer
from math import log, floor
from time import sleep
import struct

INSERT_CMD_FMTS = "i %d %d %s\n"
DELETE_CMD_FMTS = "d %d %d\n"
FIND_CMD_FMTS = "f %d %d\n"
UPDATE_CMD_FMTS = "u %d %d %s\n"
OPEN_CMD_FMTS = "o %s\n"
QUIT_CMD_FMTS = "q\n"
BEGIN_CMD_FMTS = "b\n"
ABORT_CMD_FMTS = "a\n"
COMMIT_CMD_FMTS = "m\n"
EXIT_CMD_FMTS = "e\n"
FIND_RESULT_FMTS = "Key: %d, Value: %s"
NOT_FOUND_RESULT = "Not Exists"
RESULT_FMTS = "Result: %d/%d (%.2f) %.2f secs"

TARGET_DB_NAME = 'DATA1'
BACKUP_DB_NAME = 'DATA1.bak'
EXECUTABLE_NAME = 'main'
LOG_NAME = 'Log'


def test_case():
    os.system('rm -f ' + TARGET_DB_NAME)
    os.system('cp ' + BACKUP_DB_NAME + ' ' + TARGET_DB_NAME)
    os.system('rm -f ' + LOG_NAME)
    sleep(0.1)
    f = open("log.txt", "w")
    p = Popen("./" + EXECUTABLE_NAME, stdin=PIPE, stdout=PIPE)
    succ = 0;
    
    arr = list(range(2**16))
    shuffle(arr)

    commited_arr = arr[:2 ** 14]
    aborted_arr = arr[2 ** 14:2 ** 15]
    uncommited_arr = arr[2 ** 15:]

    input_d = OPEN_CMD_FMTS % (TARGET_DB_NAME)
    f.write(input_d)
    p.stdin.write(input_d.encode("utf-8"))
    p.stdin.flush()

    p.stdout.readline()

    # commited update

    
    f.write(BEGIN_CMD_FMTS)
    p.stdin.write(BEGIN_CMD_FMTS.encode("utf-8"))
    p.stdin.flush()

    for i in commited_arr:
        input_d = UPDATE_CMD_FMTS % (1, i, "update_" + str(i))
        f.write(input_d)     
        p.stdin.write(input_d.encode("utf-8"))
        p.stdin.flush()
   
    f.write(COMMIT_CMD_FMTS)
    p.stdin.write(COMMIT_CMD_FMTS.encode("utf-8"))
    p.stdin.flush()   
#
#    sleep(180)
    p.stdout.readline() # commit wait

    f.write(BEGIN_CMD_FMTS)
    p.stdin.write(BEGIN_CMD_FMTS.encode("utf-8"))
    p.stdin.flush()

    for i in aborted_arr:
        input_d = UPDATE_CMD_FMTS % (1, i, "update_" + str(i))
        f.write(input_d)     
        p.stdin.write(input_d.encode("utf-8"))
        p.stdin.flush()
    
    f.write(ABORT_CMD_FMTS)
    p.stdin.write(ABORT_CMD_FMTS.encode("utf-8"))
    p.stdin.flush()   
#
#    sleep(10)
    p.stdout.readline() # abort wait

#    f.write(BEGIN_CMD_FMTS)
#    p.stdin.write(BEGIN_CMD_FMTS.encode("utf-8"))
#    p.stdin.flush()
#
#    for i in uncommited_arr:
#        input_d = UPDATE_CMD_FMTS % (1, i, "update_" + str(i))
#        f.write(input_d)     
#        p.stdin.write(input_d.encode("utf-8"))
#        p.stdin.flush()
#    
#    sleep(10)
    p.kill()
    f.close()

    f = open("log_2.txt", 'w')


    p = Popen("./" + EXECUTABLE_NAME, stdin=PIPE, stdout=PIPE)

    input_d = OPEN_CMD_FMTS % (TARGET_DB_NAME)
    f.write(input_d)
    p.stdin.write(input_d.encode("utf-8"))
    p.stdin.flush()
    p.stdout.readline()
    com_fail = 0
    abo_fail = 0
    unc_fail = 0
    print("COMMITED VALIDATE")

    for i in commited_arr:
        input_d = FIND_CMD_FMTS % (1, i)
        f.write(input_d)     
        p.stdin.write(input_d.encode("utf-8"))
        p.stdin.flush()
        result = p.stdout.readline().decode('utf-8').strip()
        
        if (result == (FIND_RESULT_FMTS % (i, 'update_' + str(i))).strip()):
            succ += 1
        else:
            com_fail += 1
#            print("Result" + result)
#            print("Expect" + (FIND_RESULT_FMTS % (i, 'update_' + str(i))).strip())

    print(com_fail)
    print("ABORTED VALIDATE")   
    for i in aborted_arr:
        input_d = FIND_CMD_FMTS % (1, i)
        f.write(input_d)     
        p.stdin.write(input_d.encode("utf-8"))
        p.stdin.flush()
        result = p.stdout.readline().decode('utf-8').strip()
        
        if (result == (FIND_RESULT_FMTS % (i, 'a' + str(i))).strip()):
            succ += 1  
        else:
            abo_fail +=1
#            print("Result" + result)
#            print("Expect" +(FIND_RESULT_FMTS % (i, 'a' + str(i))).strip())

    print(abo_fail)
    print("UNCOMMITED VALIDATE")   

    for i in uncommited_arr:
        input_d = FIND_CMD_FMTS % (1, i)
        f.write(input_d)     
        p.stdin.write(input_d.encode("utf-8"))
        p.stdin.flush()
        result = p.stdout.readline().decode('utf-8').strip()
        
        if (result == (FIND_RESULT_FMTS % (i, 'a' + str(i))).strip()):
            succ += 1  
        else:
            unc_fail +=1
#            print("Result" + result)
#            print("Expect" + (FIND_RESULT_FMTS % (i, 'a' + str(i))).strip())

    print(unc_fail)
    p.stdin.write(QUIT_CMD_FMTS.encode('utf-8'))
    p.kill()
    f.close()

    return succ

os.system('make clean > /dev/null')
os.system('make > /dev/null')

result = test_case()

print(RESULT_FMTS % (result, 2 ** 16, float(result)/(2 ** 16) * 100, 0))
