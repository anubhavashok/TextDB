__author__ = 'anubhav'
import requests
import hashlib
import urllib
import Cursor

class TextDB():
    endpoint = None
    key = None
    exp = None

    def __init__(self, _endpoint="http://localhost:3001", username=None, password=None):
        sha256 = hashlib.sha256()
        self.endpoint = _endpoint
        if username and password:
            r = requests.get(self.endpoint + "/auth?username={0}&password={1}".format(username,
                                                                                      sha256.update(password).digest()))
            self.key = r.text
        else:
            self.key = None

    def get(self, doc):
        safe_name = urllib.quote_plus(doc)
        return Cursor.Cursor(self.endpoint + "/get/{0}".format(safe_name), self.key)

    def list(self):
        r = requests.get(self.endpoint + "/list")
        return r.json()

    def add(self, name, textdoc):
        if not all(not c.isdigit() for c in name):
            print "Name ({0}) must be non-numeric characters".format(name)
            return
        f = open(textdoc)
        text = f.read()
        safe_text = urllib.quote_plus(text)
        name = urllib.quote_plus(name)
        r = requests.post(self.endpoint + "/add?name={0}&text={1}".format(name, safe_text))
