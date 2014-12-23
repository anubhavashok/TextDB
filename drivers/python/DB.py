__author__ = 'anubhav'
import requests
import hashlib
import urllib
import Cursor

class TextDB():
    endpoint = None
    key = None
    exp = None
    h = hashlib.sha256()

    def __init__(self, _endpoint, username=None, password=None):
        self.endpoint = _endpoint
        if username and password:
            r = requests.get(self.endpoint + "/auth?username={0},password={1}".format(username,
                                                                                      self.h.update(password).digest()))
        else:
            r = requests.get(self.endpoint + "/auth")
        self.key = r.key

    def get(self, doc):
        safe_name = urllib.quote_plus(doc)
        return Cursor.Cursor(self.endpoint + "/get/{0}".format(safe_name))