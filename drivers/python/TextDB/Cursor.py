__author__ = 'anubhav'
import requests
import urllib
import json
class Cursor():
    endpoint = None
    collection = None
    name = None
    key = None
    # Expiration date of key
    exp = None
    ptr = None
    sentencePtr = 0

    def __init__(self, _endpoint, _collection, _name, key=None):
        self.endpoint = _endpoint
        self.collection = _collection
        self.name = _name
        self.key = key

    def __repr__(self):
        return "<Cursor: file %s from collection %s>" % (self.name, self.collection)

    @staticmethod
    def num_words(s):
        return len(s.split(' '))

    def next_sentence(self, strip=True):
        """Get the next sentence of the document.

        Does what you think it does.

        Returns:
            string: Next sentence

        Examples:
            >>> a = db.get('a')
            >>> a.get_next_sentence()
            "This is the first sentence of a."
            >>> a.get_next_sentence()
            "This is the second sentence of a."

        """
        if self.sentencePtr >= self.get_size():
            return None
        r = requests.get(self.endpoint + "/sentence/{0}/{1}/{2}".format(self.collection, self.name, self.sentencePtr))
        text = urllib.unquote(r.text)
        # NOTE: the plus 1 is a hack to ignore getting the '.' in the next sentence since textdb considers puntuation
        # as words as well
        self.sentencePtr += self.num_words(text) + 1
        if strip:
            return text.strip().replace('\n', '')
        return text

    def frequency(self):
        """Get the word frequency table of document.

        TODO: Fetches the word frequency table by making a GET request to TextDB server.
        No precaching yet.

        Returns:
            string: dict containing the word frequency

        Examples:
            >>> a = db.get('a')
            >>> a.get_word_frequency()
            {'word1': 10, 'word2': 11}

        """
        r = requests.get(self.endpoint + "/termfrequency/{0}/{1}".format(self.collection, self.name))
        return json.loads(urllib.unquote(r.text))

    def text(self, start=0, limit=0):
        """Get the text in the document.

        Makes a get request to the TextDB server requesting for the text document.
        TODO: add start and limit functionality to enable user chunking of docs.

        Args:
            start (string): start word
            limit (string): number of words to get
        Returns:
            string: The text document

        Examples:
            >>> a = db.get('a')
            >>> a.get_text()
            "This is document a"

        """
        r = requests.get(self.endpoint + "/get/{0}/{1}".format(self.collection, self.name))
        return urllib.unquote(r.text)

    def sentiment(self):
        """Get the sentiment score of the document.

        Makes a GET request to the TextDB server which calculates sentiment score of the document specified.

        Returns:
            float: sentiment score

        Examples:
            >>> a = db.get('a')
            >>> a.get_sentiment()
            0.2156

        """
        r = requests.get(self.endpoint + "/sentiment/{0}/{1}".format(self.collection, self.name))
        return float(urllib.unquote(r.text))

    def size(self):
        """Get size of the document.

        Makes a get request to TextDB server and returns the number of words in the document specified.

        Returns:
            int: number of words in document

        Examples:
            >>> a = db.get('a')
            >>> a.get_size()
            102534

        """
        r = requests.get(self.endpoint + "/size/{0}/{1}".format(self.collection, self.name))
        return int(urllib.unquote(r.text))

    def similarity(self, cursor):
        if type(cursor) is not Cursor:
            print "Argument has to be of type Cursor"
            return
        if cursor.collection is not self.collection:
            print "Document {0} has to be from same collection as document {1}".format(cursor.name, self.name)
            return
        r = requests.get(self.endpoint + "similarity/{0}/{1}/{2}".format(self.collection, self.name, cursor.name))
        print r.text
        return float(r.text)