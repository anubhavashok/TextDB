__author__ = 'anubhav'

import Cursor
import requests
import urllib
import urllib2

class Collection:
    endpoint = None
    collection = None
    key = None

    def __init__(self, endpoint, collection, key=None):
        self.endpoint = endpoint
        self.collection = collection
    
    def __repr__(self):
        return "<Collection %s>" % (self.collection)

    def get(self, name):
        """Gets a cursor for document.

        Returns a cursor containing the url endpoint to access the doc that it references.
        NOTE: does not actually get anything at this point.

        Args:
          name (string): The name of the document to retrieve

        Returns:
          Cursor: The DB cursor for the requested document

        Examples:
            >>> db.list()
            ['a', 'b', 'c']
            >>> c = db.get('a')
            >>> c
            <Cursor.Cursor>

        """
        l = self.list()
        if name not in l:
            print "WARNING! Document not found"
            return None
        safe_name = urllib.quote(name)
        return Cursor.Cursor(self.endpoint, self.collection, safe_name, self.key)

    def list(self):
        """Lists all documents in DB.

        Makes a GET request to TextDB server to get names of all available documents.

        Returns:
          list: A list containing strings of

        Examples:
            >>> db.list()
            ['a', 'b', 'c']

        """
        r = requests.get(self.endpoint + "/list/{0}".format(self.collection))
        return r.json()

    def add(self, name, d):
        """Add a new text document to TextDB.

        Makes a POST request to TextDB server with document encoded in URL.

        Args:
          name (string): The name of the document to store (Non-numeric chars only)
          f (file): The file that contains the document
          s (string, optional): The string that contains the document

        Examples:
            >>> f = open('../d.txt')
            >>> db.add('d', f)
            >>> db.list()
            ['a', 'b', 'c', 'd']

            >>> db.add('d', str="This is document d")
            >>> db.list()
            ['a', 'b', 'c', 'd']
            >>> db.get('d').get_text()
            "This is document d"

        """
        if not all(not c.isdigit() for c in name):
            print "Name ({0}) must be non-numeric characters".format(name)
            return
        if type(d) == str:
            text = d
        elif type(d) == file:
            text = d.read()
        else:
            print "Error! No document provided"
            return
        #req = urllib2.Request(self.endpoint, text)
        #response = urllib2.urlopen(req)

        safe_text = urllib.quote(str(text))
        name = urllib.quote(name)
        r = requests.post(self.endpoint + "/add/{0}/{1}/{2}".format(self.collection, name, safe_text))
        return r.text

    def remove(self, name):
        """Removes an existing text document from TextDB.

        Makes a POST request to TextDB server with name.
        NOTE: does not remove words from word index yet.

        Args:
          name (string): The name of the document to store (Non-numeric chars only)

        Returns:
          boolean: indicating whether remove was successful or not

        Examples:
            >>> db.list()
            ['a', 'b', 'c']
            >>> db.remove('a')
            >>> db.list()
            ['b', 'c']

        """
        r = requests.post(self.endpoint + "/remove/{0}/{1}".format(self.collection, name))
        return True if (r.text == "1") else False

    def disk_size(self):
        r = requests.get(self.endpoint + "/size/{0}".format(self.collection))
        print "disk_size: " + r.text
        return int(r.text)

    def mark_sentiment(self, name, tag):
        r = requests.get(self.endpoint + "/marksentiment/{0}/{1}/{2}".format(self.collection, name, tag))
        return True

    def train_sentiment(self):
        r = requests.get(self.endpoint + "/trainsentiment/{0}".format(self.collection))
