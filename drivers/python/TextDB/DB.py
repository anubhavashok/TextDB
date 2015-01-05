__author__ = 'anubhav'
import requests
import hashlib
import urllib
import Cursor
import Collection


class TextDB():
    endpoint = None
    key = None
    exp = None

    def __init__(self, _endpoint="http://localhost:3001", username=None, password=None):
        try:
            r = requests.get("http://localhost:3001/")
        except Exception:
            print "Error! TextDB is not running"
            raise
        
        sha256 = hashlib.sha256()
        self.endpoint = _endpoint
        if username and password:
            r = requests.get(self.endpoint + "/auth?username={0}&password={1}".format(username,
                                                                                      sha256.update(password).digest()))
            self.key = r.text
        else:
            self.key = None

    def list(self):
        r = requests.get("{0}/{1}".format(self.endpoint, "list"))
        return r.json()

    def __getattr__(self, item):
        collections = self.list()
        if item not in collections:
            print "WARNING! collection doesn't exist"
            return None
        return Collection.Collection(self.endpoint, item, self.key)