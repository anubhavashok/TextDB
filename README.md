TextDB
======

TextDB is a database to store and query text.
It is a highly efficient and stores text files in a highly compressed format.
It aims to achieve speed and performance when processing a high load of queries

It compresses text documents by first creating a text index which stores all unique words found in the document and then subsequently using snappy (compression lib by google) to furhter compress the db.
It also has a growing index which basically means that the space alloted to each number that stores the index grows as the number of unique words grows

It is coupled with a contextual indexer which allows you to query the words surrounding a particular word.
This allows one to determine what context the word was used.

Lastly, it removes all punctuation and converts all characters to lower case and works only for english based languages.

CONTEXTUAL INDEXER
A contextutal indexer pre-indexes each word in each corpus and creates an index containing the position of the word in the documents it was found in.
The user can then conduct queries pertaining to the context of the word.
There are two main query forms but they are to be expanded in the future.
The first possible query is the proximity query while the second one is the window query.
The proximity query is a efficiently scans the surrounding words and adds to the convenience of the user.

PROMIXTY QUERY
It takes in a pair of words and a left and right proximity number and returns a boolean i.e. (w1, w2, l, r) -> t/f
It checks if w2 is within l words left and r words right of w1.

This is done in an efficient way as the words and their positions in the document are pre-indexed.
Caches will be added in subsequent versions to further speed up the search.
