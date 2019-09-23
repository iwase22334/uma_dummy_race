import os
import requests
import json

class DummyRace:
    def __init__(self):
        self.__dummy_race_server_url = os.environ['DUMMY_RACE_URL']

    @classmethod
    def __generate_query_tansyo(cls, id, vote_list):
        query = '{ "race_id": { "year":"%s", "monthday":"%s", "jyocd":"%s", "kaiji":"%s", "nichiji":"%s", "racenum":"%s" },' % (id)
        query = query + ' "tansyo_vote": [ '

        first = True
        for umaban, hyosu in vote_list:
            if first:
                first = False
                query = query + ' { "umaban": "%s", "hyosu": %d } ' % (umaban, hyosu)
            else:
                query = query + ' , { "umaban": "%s", "hyosu": %d } ' % (umaban, hyosu)

        query = query + ' ] }'

        return query.strip()

    # @param id [ 'year', 'monthday', 'jyocd', 'kaiji', 'nichiji', 'racenum']
    def vote_tansyo(self, id, vote_list):

        query = DummyRace.__generate_query_tansyo(id, vote_list)
        r = requests.post(self.__dummy_race_server_url, query)

        if r.status_code == 200:
            body = json.loads(r.text)
            return body["payout"]

        else:
            body = json.loads(r.text)
            raise RuntimeError("Dummy race server says: %s" % (body["error"],))
