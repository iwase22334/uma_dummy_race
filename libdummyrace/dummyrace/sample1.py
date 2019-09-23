#import dummyrace
import psycopg2
import os
import sys
import dummyrace

query = '''SELECT year, monthday, jyocd, kaiji, nichiji, racenum \
from n_race \
where concat(year, monthday) > '20000000' and datakubun='7' \
order by year asc, monthday asc limit 1'''.strip()

print (query)

try:
    connection= psycopg2.connect(os.environ.get('DATABASE_URL_SRC'))
except Exception as e:
    print('psycopg2: opening connection 01 faied: %s' % e)
    sys.exit(0)

with connection.cursor() as cur:
    cur.execute(query)
    id = cur.fetchone()

connection.close()

vote_list = [("01", 10), ("06", 20)]

print(id)
print(vote_list)

race = dummyrace.DummyRace()
res = race.vote_tansyo(id, vote_list)
print(res)

