// const http = require('http');

// const hostname = '192.168.1.95';
// const port = 3000;

const { MongoClient } = require('mongodb');


async function listDatabases(client){
    databasesList = await client.db().admin().listDatabases();

    console.log("Databases:");
    databasesList.databases.forEach(db => console.log(` - ${db.name}`));
};




async function createData(client, newData){
    const result = await client.db("powerdata").collection("power").insertOne(newData);
    console.log(`New data inserted with the following id: ${result.insertedId}`);
}






async function findDataTimeGreaterThan(client, {
    minimumunixtime = 0,
//    maximumNumberOfResults = Number.MAX_SAFE_INTEGER
} = {}) {

    // See https://mongodb.github.io/node-mongodb-native/3.6/api/Collection.html#find for the find() docs
    const cursor = client.db("powerdata").collection("power")
        .find({
            unixtime: { $gte: minimumunixtime }
        }
        )
    // Store the results in an array
    const results = await cursor.toArray();
        var data = "Time, power (VA), unixtime";
    // Print the results
    if (results.length > 0) {
        console.log(`Found listing(s) with at least ${minimumunixtime} time:`);
        results.forEach((result, i) => {
           const date = new Date(result.timestamp);

            console.log();
            console.log(`${i + 1}. power: ${result.power}`);
            console.log(`   _id: ${result._id}`);
            console.log(`   time: ${result.unixtime}`);
            console.log(`   Date: ${result.timestamp}`);
            data+="\r\n"+ result.timestamp+"," + result.power +"," + result.unixtime
        });
        require("fs").writeFileSync("powerdata.CSV", data)
    } else {
        console.log(`No listings found with at least ${minimumNumberOfBedrooms} bedrooms and ${minimumNumberOfBathrooms} bathrooms`);
    }
}







// server.js

var express = require('express');

var app = express();

var PORT = 3000;

const uri = "mongodb+srv://bcouraud:Topologie1@clusterbc.6p3batc.mongodb.net?retryWrites=true&w=majority";
const client = new MongoClient(uri);


app.get('/', function(req, res) {
  res.status(200).send('Hello world');
});

app.get('/sendData', function(req, res) {
  var newpower = req.query.power;
  var name = req.query.name;
  console.log('power:'+newpower);

  client.connect();
  var now = new Date();

  createData(client,
      {
          power: newpower,
          timestamp: now,
          unit: "VA",
          unixtime: Math.floor(Date.now() / 1000)
      }
  );
  findDataTimeGreaterThan(client, {
      minimumunixtime: 0
  }); 


   res.status(200).send('received Data: power : ' +newpower + ' and name = ' +name);



});


app.listen(PORT, function() {
  console.log('Server is running on PORT:',PORT);
});