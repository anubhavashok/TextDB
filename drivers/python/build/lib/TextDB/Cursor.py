__author__ = 'anubhav'
import requests
import urllib
class Cursor():
    endpoint = None
    key = None
    # Expiration date of key
    exp = None
    ptr = None
    sentencePtr = 0

    def __init__(self, _endpoint, key=None):
        self.endpoint = _endpoint
        self.key = key

    @staticmethod
    def num_words(s):
        return len(s.split(' '))

    def get_next_sentence(self, strip=True):
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
        r = requests.get(self.endpoint + "/sentence/{0}".format(self.sentencePtr))
        text = urllib.unquote(r.text)
        # NOTE: the plus 1 is a hack to ignore getting the '.' in the next sentence since textdb considers puntuation
        # as words as well
        self.sentencePtr += self.num_words(text) + 1
        if strip:
            return text.strip().replace('\n', '')
        return text

    def get_word_frequency(self):
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
        r = requests.get(self.endpoint + "/wordFrequency")
        return urllib.unquote(r.text)

    def get_text(self, start=0, limit=0):
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
        r = requests.get(self.endpoint)
        return urllib.unquote(r.text)

    def get_sentiment(self):
        """Get the sentiment score of the document.

        Makes a GET request to the TextDB server which calculates sentiment score of the document specified.

        Returns:
            float: sentiment score

        Examples:
            >>> a = db.get('a')
            >>> a.get_sentiment()
            0.2156

        """
        r = requests.get(self.endpoint + "/sentiment")
        return float(urllib.unquote(r.text))

    def get_size(self):
        """Get size of the document.

        Makes a get request to TextDB server and returns the number of words in the document specified.

        Returns:
            int: number of words in document

        Examples:
            >>> a = db.get('a')
            >>> a.get_size()
            102534

        """
        r = requests.get(self.endpoint + "/size")
        return int(urllib.unquote(r.text))