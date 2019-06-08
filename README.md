# uma_dummy_race
## API
### Request
```
{
    "race_id": {
        "year": "2018",
        "monthday":"1001",
        "jyocd":"01",
        "kaiji":"01",
        "nichiji": "01",
        "racenum": "01"
    },
    "tansyo_vote": [
        { "umaban": "01", "hyosu": 20 },
        { "umaban": "05",  "hyosu": 4 }
    ],
    "hukusyo_vote": [
        { "umaban": "03",  "hyosu": 13 },
        { "umaban": "05",  "hyosu": 1 },
        { "umaban": "14",  "hyosu": 2 }
    ],
    "wakuren_vote": [
        { "kumiban": "58",  "hyosu": 1 },
        { "kumiban": "22",  "hyosu": 1 }
    ],
    "umaren_vote": [
        { "kumiban": "0413",  "hyosu": 1 }
    ],
    "wide_vote": [
        { "kumiban": "1318",  "hyosu": 1 }
    ],
    "umatan_vote": [
        { "kumiban": "0108",  "hyosu": 1 }
    ],
    "sanrenpuku_vote": [
        { "kumiban": "010812",  "hyosu": 1 }
    ]
}
```
### Response
```
{
    "payout": 2000
}
```
### Error response
```
{
    "error": "Record not found"
}
```
