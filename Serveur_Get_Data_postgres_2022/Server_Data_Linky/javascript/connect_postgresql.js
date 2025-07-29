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

const { Client } = require('pg');

const clientpostgre = new Client({
    user: 'pi',
    host: '192.168.1.95',
    database: 'powerdata',
    password: 'Topologie1',
    port: 5432,
});

clientpostgre.connect();


const querySelect = `
SELECT * FROM power;
`;



var now = new Date();
var power = 19;
var unixtime = Math.floor(Date.now() / 1000);
const queryInsert = `
INSERT INTO power VALUES (to_timestamp(${Date.now()} / 1000.0), ${power},${unixtime});`;


// clientpostgre.query(queryInsert, (err, res) => {
//     console.log(queryInsert)
//     if (err) {
//         console.error(err);
//         return;
//     }
//     console.log('Data insert successful');
//     clientpostgre.end();
// });



const fs = require('fs');
 
const readline = require("readline");
const stream = fs.createReadStream("powerdata copy.csv");
const rl = readline.createInterface({ input: stream });
let data = [];
 
rl.on("line", (row) => {
    // clientpostgre.connect();

    let data = [];

    data.push(row.split(","));
    // Timestamp in seconds
    let unixTimestamp = data[0][3];
    let power = data[0][2];
    /* Create a new JavaScript Date object based on Unix timestamp.
    Multiplied it by 1000 to convert it into milliseconds */
    let date = new Date(unixTimestamp * 1000);

    
let queryInsert2 = `
INSERT INTO power VALUES (to_timestamp(${unixTimestamp} / 1000.0), ${power},${unixTimestamp});`;
clientpostgre.query(queryInsert2);
// console.log("ok")
// clientpostgre.query(queryInsert, (err, res) => {
//     console.log(queryInsert)
//     if (err) {
//         console.error(err);
//         return;
//     }
//     console.log('Data insert successful');
//    clientpostgre.end();
// });

}
);

// clientpostgre.query(queryInsert, (err, res) => {
//     console.log(queryInsert)
//     if (err) {
//         console.error(err);
//         return;
//     }
//     console.log('Data insert successful');
//    clientpostgre.end();
// });

rl.on("close", () => {

    console.log(data);
});
// clientpostgre.query(querySelect, (err, res) => {
//     console.log("here")
//     if (err) {
//         console.error(err);
//         return;
//     }
//     for (let row of res.rows) {
//         console.log(row);
//     }
//     clientpostgre.end();
// });

