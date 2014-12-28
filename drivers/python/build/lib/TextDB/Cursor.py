__author__ = 'anubhav'
import requests
import urllib
class Cursor():
    endpoint = None
    key = None
    # Expiration date of key
    exp = None
    ptr = None
    sentenceNumber = None

    def __init__(self, _endpoint, key=None):
        self.endpoint = _endpoint
        self.key = key

    def get_next_sentence(self):
        r = requests.get(self.endpoint + "/sentence/{0}".format(self.sentenceNumber))
        return urllib.unquote_plus(r.text)

    def get_word_frequency(self):
        r = requests.get(self.endpoint + "/wordFrequency")
        return urllib.unquote_plus(r.table)

    def get_text(self):
        r = requests.get(self.endpoint)
        return urllib.unquote_plus(r.text)