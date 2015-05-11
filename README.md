# TextDB

www.textedb.com

Terms to note
widx - bitvector containing index of corresponding word


Things to note
1. All reads and writes go through raft consensus.
2. Query functions are lambdas to provide extensibility. (Real time custom operations)
3. LRU works in conjunction with db's memory management, not to be used standalone.
4. Similarity.h contains all the text mining functions for now.

UNUSED
oplog.cpp
oplog.h
proposer.cpp
proposer.h
acceptor.cpp
acceptor.h	
