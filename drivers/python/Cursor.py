__author__ = 'anubhav'
import requests
class Cursor():
    endpoint = None
    key = None
    # Expiration date of key
    exp = None
    ptr = None
    sentenceNumber = None

    def __init__(self, _endpoint):
        self.endpoint = _endpoint

    def get_next_sentence(self):
        r = requests.get(self.endpoint + "/sentence/{0}".format(self.sentenceNumber))
        return r.text

    def get_word_frequency(self):
        r = requests.get(self.endpoint + "/wordFrequency")
        return r.table
