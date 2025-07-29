//configuration:
//sudo apt-get dist-upgrade
//curl -sL https://deb.nodesource.com/setup_14.x | sudo -E bash -
//sudo apt-get install -y nodejs



// install grafana:
//https://grafana.com/tutorials/install-grafana-on-raspberry-pi/

//install local postgresql:
//https://pimylifeup.com/raspberry-pi-postgresql/
//mdp: Topologie1
//then:
//https://stackabuse.com/using-postgresql-with-nodejs-and-node-postgres/

//Install Local MySQL


//node driver for MySQL
//npm install --save pg
// const { Client } = require('pg');

// const client = new Client({
//     user: 'postgres',
//     host: 'localhost',
//     database: 'testdb',
//     password: '1234abcd',
//     port: 5432,
// });

// client.connect();


//Create a local DB:
//in mongo shell:
//use powerdata
//db
// db.collection.insert({})
//show dbs



// to run the code:
// open the file with Code.
// in the terminal from code: npm i
// then in the terminal: node index_Mongo_Atlas.js

// then in a browser: http://192.168.1.95:3000/sendData?power=5&name=puissance


// const http = require('http');

// const hostname = '192.168.1.95';
// const port = 3000;
//, Code





// creation of SQL server:
// sudo su postgres
// createuser pi -P --interactive
// mdp:Topologie1
// CREATE DATABASE pi;
// exit
// psql
// CREATE DATABASE powerdata;
// \connect powerdata;
// CREATE TABLE power (time timestamp, powerVA int8, unixtime bigint);
// INSERT INTO power VALUES ('23/08/2022 12:27:00',18,1661250420);
// SELECT * FROM power;

// needed to configure PostreSQL server to accept remote requests:
// https://www.netiq.com/documentation/identity-manager-47/setup_windows/data/connecting-to-a-remote-postgresql-database.html
//nano postgresql.conf
// add listen_addresses = '*'
// nano pg_hba.conf
// add: host all all 0.0.0.0/0 md5



// then grafana connection: 
// PostgreSQL Connection
// Host
// 192.168.1.95:5432
// Database // powerdata
// User // pi
// Password: Topologie1
// configured

// Reset
// TLS/SSL Mode: // disable
const fs = require('fs');

const { Client } = require('pg');

const clientpostgre = new Client({
    user: 'pi',
    host: '192.168.1.95',
    database: 'powerdata',
    password: 'Topologie1',
    port: 5432,
});

clientpostgre.connect();

const {MongoClient} = require('mongodb');


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
            //const date = new Date(result.timestamp);
            console.log();
            console.log(`${i + 1}. power: ${result.power}`);
            console.log(`   _id: ${result._id}`);
            console.log(`   time: ${result.unixtime}`);
            console.log(`   Date: ${result.timestamp}`);
            data+="\r\n"+ result.timestamp+"," + result.power +"," + result.unixtime
        });
        require("fs").writeFileSync("powerdata.CSV", data)
    } else {
        console.log(`No listings found`);
    }
}







// server.js

var express = require('express');

var app = express();

var PORT = 3000;

const uriAtlas = "mongodb+srv://bcouraud:Topologie1@clusterbc.6p3batc.mongodb.net?retryWrites=true&w=majority";
const clientAtlas = new MongoClient(uriAtlas);
// const urilocal = "mongodb://localhost:27017";
// const clientlocal = new MongoClient(urilocal);

app.get('/', function(req, res) {
  res.status(200).send('Hello world');
});


app.get('/sendData', function(req, res) {
  var newpower = req.query.power;
  var name = req.query.name;
  console.log('power:'+newpower);

  clientAtlas.connect();
  var now = new Date();

//   createData(clientAtlas,
//       {
//           power: newpower,
//           timestamp: now,
//           unit: "VA",
//           unixtime: Math.floor(Date.now() / 1000)
//       }

//   );
//   
//store in local csv
var now = new Date();
var data = "\r\n"+ now.toLocaleString()+"," + newpower +"," + Math.floor(Date.now() / 1000);

fs.appendFile("powerdata.CSV", data, "utf-8", (err) => {
    if (err) console.log(err);
    else console.log("Data saved");
  });

  var unixtime = Math.floor(Date.now() / 1000);
  var queryInsert = `
  INSERT INTO power VALUES (to_timestamp(${unixtime} / 1000.0), ${newpower},${unixtime});`;
  clientpostgre.query(queryInsert);

//   clientpostgre.query(queryInsert, (err, res) => {
//     console.log(queryInsert)
//     if (err) {
//         console.error(err);
//         return;
//     }
//     console.log('Data insert successful');
//    clientpostgre.end();
// });




   res.status(200).send('received Data: power : ' +newpower + ' and name = ' +name);
});


app.listen(PORT, function() {
  console.log('Server is running on PORT:',PORT);
});