const { MongoClient } = require('mongodb');
const client = new MongoClient("mongodb://localhost:27017");
client.connect();