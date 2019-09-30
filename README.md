# uma_dummyrace
## API
### Request
Generate POST query as following format
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
    ]
```

### Response

#### 200 OK
Return race result as following format in response body
```
{
    "payout": 2000
}
```

#### 400 Bad Request
* race_id or vote format is invalid
```
{
    "error": "Invalid request format"
}
```

* No valid data found about the requested id in database 
```
{
    "error": "Record not found"
}
```
