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

    def drop(self, collection):
        """Drops a particular collection from TextDB.

        Makes a POST request to TextDB server.

        Args:

        Examples:
            >>> db.list()
            ['a', 'b', 'c', 'd']
            >>> db.drop('d')
            >>> db.list()
            ['a', 'b', 'c']

        """
        r = requests.post("{0}/{1}/{2}".format(self.endpoint, "drop", collection))
        return r.text

    def create_collection(self, collection, encoding):
        """Creates a new collection in TextDB.

        Makes a POST request to TextDB server.

        Args:

        Examples:
            >>> db.list()
            ['a', 'b', 'c', 'd']
            >>> db.create_collection('1')
            >>> db.list()
            ['1', 'a', 'b', 'c']

        """
        if not collection.isalpha():
            print "Collection name has to be alphabetical"
        else:
            r = requests.post("{0}/{1}/{2}/{3}".format(self.endpoint, "create", collection, encoding))
            return r.text