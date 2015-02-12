__author__ = 'anubhav'

import glob
import os
import time


def comparator(r1, r2):
    try:
        t1 = int(r1.split('.')[-1])
        t2 = int(r2.split('.')[-1])
        # reverse because larger timestamp means more recent
        return t2 - t1
    except ValueError:
        print "Wrong remove file format"

def get_size(glob):
    sum = 0
    for g in glob:
        sum += os.path.getsize(g)
    return sum


# remove script
# A cron which checks for exceeding disk space every second
# removes doc files marked with remove, starting with least timestamp

path = "/Users/anubhav/TextDB/data" # first argument passed
wait = 1 # second argument passed (in seconds) OR 1 second
allowed = 64000 # bytes
space =  sum([os.path.getsize(f) for f in os.listdir(path) if os.path.isfile(f)])
epsilon = 10 #mb

while True:
    print space
    removes = glob.glob(path + '/**/**/files/*remove*')
    removes_size = get_size(removes)
    print removes_size
    if allowed - removes_size < epsilon:
        print "Need more space"
        if len(removes) == 0:
            print "But no more files to remove"
        else:
            removes.sort(comparator)
            remove = removes[0]
            print "removing", remove
            os.remove(remove)
            removes = removes[1:]
    space = sum([os.path.getsize(f) for f in os.listdir(path) if os.path.isfile(f)])
    time.sleep(wait)


