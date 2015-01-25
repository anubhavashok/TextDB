import sys
sys.path.append("../")

from DB import *
import os
import unittest


class TestSequenceFunctions(unittest.TestCase):
    db = None
    c = None

    def setUp(self):
        self.db = TextDB()
        self.c = self.db.c

    def test_create_and_remove(self):
        self.db.create_collection("a", "unicode")
        l = self.db.list()
        self.assertTrue("a" in l)
        self.db.drop("a")
        l = self.db.list()
        self.assertFalse("a" in l)

    # def test_collection_doesnt_exist(self):
    #     self.assertEqual(self.db.abcde, None)

    def test_collection_exists(self):
        self.assertNotEqual(self.c, None)

    def test_add(self):
        text = "wow this works"
        self.c.add("abc", text)
        d = self.c.get("abc")
        # get text
        self.assertEqual(d.get_text(), text)
        self.assertEqual(d.get_next_sentence(), text)

    def test_add_and_remove(self):
        name = "toberemoved"
        self.c.add(name, "sample text")
        l = self.c.list()
        self.assertTrue(name in l)
        self.c.remove(name)
        l = self.c.list()
        self.assertFalse(name in l)

    def test_doc_doesnt_exist(self):
        self.assertEqual(self.c.get('zzzzzzzzzz'), None)

    def test_sentence(self):
        text = "wow this works. this also works"
        self.c.add('doc', text)
        e = self.c.get('doc')
        self.assertEqual(e.get_next_sentence(), text.split('.')[0].strip() + ".")
        self.assertEqual(e.get_next_sentence(), text.split('.')[1].strip())

    def test_compression(self):
        # create compressed collection
        self.db.create_collection("compr", "compressed")
        comp = self.db.compr
        self.db.create_collection("unic", "unicode")
        uni = self.db.unic
        for p in os.listdir('./files'):
            print p
            f = open("./files/" + p)
            name = os.path.basename(p).split('.')[0]
            comp.add(name, f)
            uni.add(name, f)
        # get size in bytes for both
        c_size = comp.disk_size()
        u_size = uni.disk_size()
        self.db.drop("compr")
        self.db.drop("unic")
        ratio = 100 * c_size/float(u_size)
        print str(ratio) + "%"
if __name__ == '__main__':
    unittest.main()
