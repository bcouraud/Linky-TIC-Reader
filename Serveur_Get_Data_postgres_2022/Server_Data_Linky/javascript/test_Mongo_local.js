
// before running the code, need to run the following from a windows terminal:
// cd C:\MongoData\db
// C:\MongoData\db>"C:\Program Files\MongoDB\Server\5.0\bin\mongod.exe" --dbpath "C:\MongoData\db"
//then open another prompt:
// C:\MongoData\db>"C:\Program Files\MongoDB\Server\5.0\bin\mongo.exe"

//################ raspberryPi ################
// https://pimylifeup.com/mongodb-raspberry-pi/
// https://www.mongodb.com/fr-fr/basics/create-database


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
     const uri = "mongodb://localhost:27017";
    const client = new MongoClient(uri);

// had to follow to setup a local db https://www.tutorialspoint.com/mongodb/mongodb_environment.htm#:~:text=Install%20MongoDB%20On%20Windows,-To%20install%20MongoDB&text=MongoDB%20requires%20a%20data%20folder,is%20c%3A%5Cdata%5Cdb.     

// and https://www.prisma.io/dataguide/mongodb/setting-up-a-local-mongodb-database

// then had to run mongod.exe: dans le terminal (cmd) windows:
// C:\Users\Benoit Couraud>cd C:\
// C:\>cd MongoData
// C:\MongoData>cd db

//
// C:\MongoData\db>"C:\Program Files\MongoDB\Server\5.0\bin\mongod.exe" --dbpath "C:\MongoData\db"

//then open another prompt:
// C:\MongoData\db>"C:\Program Files\MongoDB\Server\5.0\bin\mongo.exe"

// then, used MongoKompass to connect to mongoDB: mongodb://localhost:27017
// from Kompass, I created a cluster "powerdata" with a collection "power"


    try {
        // Connect to the MongoDB cluster
        await client.connect();
        var now = new Date();

        // await createData(client,
        //     {
        //         power: 19,
        //         timestamp: now,
        //         unit: "VA",
        //         unixtime: Math.floor(Date.now() / 1000)
        //     }
        // );
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


