import json
import webapp2
import google.appengine
from main import Handler

class GetData(Handler):
    def post(self):
        data = json.loads(self.request.body)
        motionValue = data["status"][0]["value"]