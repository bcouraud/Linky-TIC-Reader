// server.js

var express = require('express');
const Influx = require('influx');

var app = express();

var PORT = 3000;




//const {InfluxDB} = require('@influxdata/influxdb-client')

// You can generate an API token from the "API Tokens Tab" in the UI
const {InfluxDB} = require('@influxdata/influxdb-client')

// You can generate an API token from the "API Tokens Tab" in the UI
const token = '5a1lUVCD3C6z8thaJSUEsyz-nDTwNbdReZpUehdB85VZcHEzk9Zuy6u41fcfHbhh1HaNgvNWFPNPYcfwdDYQWA=='
const org = 'UoG'
const bucket = 'PowerMeasurement'

const client = new InfluxDB({url: 'http://localhost:8086', token: token})


// const influx = new Influx.InfluxDB({
//     host: 'localhost',
//     database: 'power_measurement',
//     schema: [
//       {
//         measurement: 'power',
//         fields: { Power: Influx.FieldType.FLOAT },
//         tags: ['unit', 'location']
//       }
//     ]
//   });


//   influx.getDatabaseNames()
//   .then(names => {
//     if (!names.includes('power_measurement')) {
//       return influx.createDatabase('power_measurement');
//     }
//   })
//   .then(() => {
//     app.listen(app.get('port'), () => {
//       console.log(`Listening on ${app.get('port')}.`);
//     });
//     writeDataToInflux(House1);
//   })
//   .catch(error => console.log({ error }));

const {Point} = require('@influxdata/influxdb-client')
const writeApi = client.getWriteApi(org, bucket)
writeApi.useDefaultTags({host: 'host1'})

const point = new Point('power1').floatField('power', 23.43234543)
writeApi.writePoint(point)

writeApi
    .close()
    .then(() => {
        console.log('FINISHED')
    })
    .catch(e => {
        console.error(e)
        console.log('Finished ERROR')
    })



app.get('/', function(req, res) {
    res.status(200).send('Hello world');
});

app.get('/sendData', function(req, res) {
    var power = req.query.power;
    var name = req.query.name;
    console.log('power:'+power);
    // res.writeHead(200, {'Content-Type': 'text/html'})
    // res.send(id)
    const writeApi = client.getWriteApi(org, bucket)
    writeApi.useDefaultTags({host: 'host1'})
    
    let point = new Point('power1').floatField('power', power)
    writeApi.writePoint(point)
    
    writeApi
        .close()
        .then(() => {
            console.log('FINISHED')
        })
        .catch(e => {
            console.error(e)
            console.log('Finished ERROR')
        })


    // influx.writePoints([
    //     {
    //       measurement: 'power',
    //       tags: {
    //         unit: 'VA',
    //         location: 'Nice',
    //       },
    //       fields: { Power: power},
    //       timestamp: Date.now(),
    //     }
    //   ], {
    //     database: 'power_measurement',
    //     precision: 's',
    //   })
    //   .catch(error => {
    //     console.error(`Error saving data to InfluxDB! ${err.stack}`)
    //   });



    res.status(200).send('received Data: power : ' +power + ' and name = ' +name);



  });


app.listen(PORT, function() {
    console.log('Server is running on PORT:',PORT);
});