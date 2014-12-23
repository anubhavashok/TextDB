"use strict";

var express = require('express');
var async = require('async');
var request = require('request');
var querystring = require('querystring');

var app = express();

// HTML initialization
app.use(express.static(process.cwd() + '/public'));
app.set('views', __dirname + '/www');
app.engine('html', require('ejs').renderFile);

var PORT_NUM = 3001;


app.get("/console", function(req, res) {
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

// TODO: backwards compatability, to transition to new format
app.get("/get", function(req, res) {
    var query = req.query;
    var name = query.name;
    console.log("Getting: " + name);
    var url = "http://localhost/get?" + name;
    request.get(url, function(error, response, body) {
        res.type('text/plain');
        res.send(body);
    });
});

app.get("/get/:name", function(req, res) {
    var name = querystring.unescape(req.params.name);
    console.log("Getting: " + name);
    var url = "http://localhost/get?" + name;
    request.get(url, function(error, response, body) {
        res.type('text/plain');
        res.send(body);
    });
});


app.get("/get/:name/getSentence/:number", function(req, res) {
    var name = querystring.unescape(req.params.name);
    var number = querystring.unescape(req.params.number);
    console.log("Getting sentence number: " + number + " from doc: " + name);
    var url = "http://localhost/get?" + name;
    request.get(url, function(error, response, body) {
        res.type('text/plain');
        res.send(body);
    });
});


app.get("/adddoc", function(req, res) {

    var query = req.query;
    var name = query.name;
    var path = query.path;
    var url = "http://localhost?adddoc&"+name+"&"+path;
    console.log(name);
    request.get(url, function(error, response, body) {
        res.send("Added: " +  name);
    });

});

app.get("/", function(req, res) {
    res.redirect("/console");
});


var server = app.listen(PORT_NUM);
console.log('Listening on port ' + server.address().port);
