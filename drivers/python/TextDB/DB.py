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
        safe_name = urllib.quote_plus(name)
        return Cursor.Cursor(self.endpoint + "/get/{0}".format(safe_name), self.key)

    def list(self):
        """Lists all documents in DB.

        Makes a GET request to TextDB server to get names of all available documents.

        Returns:
          list: A list containing strings of

        Examples:
            >>> db.list()
            ['a', 'b', 'c']

        """
        r = requests.get(self.endpoint + "/list")
        return r.json()

    def add(self, name, f, s=None):
        """Add a new text document to TextDB.

        Makes a POST request to TextDB server with document encoded in URL.
        There is probably a better way to do this..

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
        if str:
            text = s
        elif f:
            text = f.read()
        else:
            print "Error! No document provided"
            return
        safe_text = urllib.quote_plus(text)
        name = urllib.quote_plus(name)
        r = requests.post(self.endpoint + "/add?name={0}&text={1}".format(name, safe_text))
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
        r = requests.post(self.endpoint + "/remove/{0}".format(name))
        return True if (r.text == "1") else False