#!/bin/bash

data=$(cat - <<EOS
{
    "race_id": {
        "year": "2019",
        "monthday":"0303",
        "jyocd":"10",
        "kaiji":"01",
        "nichiji": "08",
        "racenum": "12"
    },
    "tansyo_vote": [
        { "umaban": "01", "hyosu": 20 },
        { "umaban": "05", "hyosu": 4 },
        { "umaban": "06", "hyosu": 1 }
    ]
}
EOS
)
curl -X POST -d "$data" -H "Connection: keep-alive" localhost:8080 -v
