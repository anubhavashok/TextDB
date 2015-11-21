from Tagger import *
from BaseHTTPServer import BaseHTTPRequestHandler
from BaseHTTPServer import HTTPServer
import urlparse



class SampleTagger(Tagger):
    
    class SampleHandler(BaseHTTPRequestHandler):
        taggerClass = None
        def do_GET(self):
            parsed_path = urlparse.urlparse(self.path)
            args = parsed_path.query
            c = parsed_path.path[1:]
            print c
            res = '1'
            # extract data, perform computation and then return result
            # Also handle start/stop state requests
            
            self.send_response(200)
            self.end_headers()
            print self.taggerClass
            res = 'Unknown'
            if c in self.taggerClass.cmds:
                res = self.taggerClass.cmds[c](args)
            self.wfile.write(res)
            return

    def compute(self, args):
        if self.status != ACTIVE:
            raise AttributeError("Current status is not active")
            return False
        return True
        pass
    
    def train(self, args):
        if self.status != TRAINING:
            raise AttributeError("Current status is not training")
            return False
        return True

    def __init__(self):
        Tagger.__init__(self, {'handler': self.SampleHandler})
        self.cmds['compute'] = self.compute
        self.cmds['train'] = self.train
        self.SampleHandler.taggerClass = self
        pass



if __name__ == '__main__':
    t = SampleTagger()
    t.run()