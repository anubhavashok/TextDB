"use strict";

var express = require('express');
var async = require('async');
var request = require('request');

var app = express();

// HTML initialization
app.use(express.static(process.cwd() + '/public'));
app.set('views', __dirname + '/www');
app.engine('html', require('ejs').renderFile);

var PORT_NUM = 3001;


app.get("/console", function(err, res) {
    async.waterfall([
        function(callback) {
            request.get("http://localhost/list/", function(error, response, body){
                console.log(body);
                callback(null, {"availableDocs": body.split("\n")});
            });
        }
    ], function(err, result) {
        res.render("index.html", result);
    });

});

var server = app.listen(PORT_NUM);
console.log('Listening on port ' + server.address().port);
