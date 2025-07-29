
var util= require('util');
var encoder = new util.TextEncoder('utf-8');
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






async function main(){
    /**
     * Connection URI. Update <username>, <password>, and <your-cluster-url> to reflect your cluster.
     * See https://docs.mongodb.com/ecosystem/drivers/node/ for more details
     */
     const uri = "mongodb+srv://bcouraud:Topologie1@clusterbc.6p3batc.mongodb.net?retryWrites=true&w=majority";
    const client = new MongoClient(uri);

    try {
        // Connect to the MongoDB cluster
        await client.connect();
        var now = new Date();

        await createData(client,
            {
                power: 555,
                timestamp: now,
                unit: "VA",
                unixtime: Math.floor(Date.now() / 1000)
            }
        );
        await findDataTimeGreaterThan(client, {
            minimumunixtime: 0
        }); 

        // Make the appropriate DB calls
       // await  listDatabases(client);

    } catch (e) {
        console.error(e);
    } finally {
        await client.close();
    }

}

main().catch(console.error);


