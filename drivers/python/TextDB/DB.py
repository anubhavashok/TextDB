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

    def __init__(self, _endpoint="http://localhost:9090/v1", username=None, password=None):
        try:
            r = requests.get(_endpoint)
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

    def __getattr__(self, item):
        collections = self.list()
        if item not in collections:
            raise AttributeError("WARNING! collection doesn't exist")
        else:
            return Collection.Collection(self.endpoint, item, self.key)

    def __getitem__(self, item):
        self.__getattr__(item)

    def __repr__(self):
        return "<TextDB at %s>" % self.endpoint

    def get(self, name):
        return self.__getattr__(name)

    def list(self):

        r = requests.get("{0}/{1}".format(self.endpoint, "list"))
        contents = r.json()
        if "collectionNames" not in contents:
            raise KeyError('collectionNames parameter not returned')
        return contents['collectionNames']

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

    def create_collection(self, collection, encoding='unicode'):
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
        encoding = encoding.lower()
        if not collection.isalpha():
            print "Collection name has to be alphabetical"
        elif (encoding != "compressed") and (encoding != "unicode"):
            print "Invalid type '%s'. Allowed types are 'compressed' or 'unicode'" % encoding
        else:
            r = requests.post("{0}/{1}/{2}/{3}".format(self.endpoint, "add", collection, encoding))
            return r.text