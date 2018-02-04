import os
import json

import webapp2

from google.appengine.api import users

class Handler(webapp2.RequestHandler):
    def respondToJson(self, json_data):
        '''
        Responds to JSON requests with data
        '''
        self.response.out.write(json.dumps((json_data)))

    def write(self, *a, **kw):
        '''
        Basic writing responses with plain text
        '''
        self.response.out.write(*a, **kw)