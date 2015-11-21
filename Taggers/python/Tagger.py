#Python base class for building a tagger

from BaseHTTPServer import BaseHTTPRequestHandler
from BaseHTTPServer import HTTPServer
import urlparse
DISABLED, STARTED, TRAINING, ACTIVE = range(4)
class Tagger(object):
    handler = None
    status = DISABLED
    
    def do_started_ack(args):
        status = STARTED
        return True
    
    def do_training_ack(args):
        status = TRAINING
        return True
    
    def do_active_ack(args):
        status = ACTIVE
        return True
    
    def do_disabled_ack(args):
        status = DISABLED
        return True

    def train(self, args=None):
        raise NotImplementedError

    # Tag based on provided features, return class/value
    def compute(self, args=None):
        raise NotImplementedError

    cmds = {
        "started_ack": do_started_ack,
        "training_ack": do_training_ack,
        "active_ack": do_active_ack,
        "disabled_ack": do_disabled_ack
    }
    def __init__(self, args=None):
        try:
            self.handler = args['handler']
        except KeyError:
            raise KeyError("args['handler'] is a required argument")
        pass
    def run(self, args=None):
        server = HTTPServer(('localhost', 8080), self.handler)
        print 'Starting server, use <Ctrl-C> to stop'
        server.serve_forever()

'''
    class GetHandler(BaseHTTPRequestHandler):
        
        def do_GET(self):
            parsed_path = urlparse.urlparse(self.path)
            res = ''
            # extract data, perform computation and then return result
            # Also handle start/stop state requests
            
            self.send_response(200)
            self.end_headers()
            self.wfile.write(res)
            return
'''