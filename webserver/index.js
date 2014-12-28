"use strict";

var express = require('express');
var async = require('async');
var request = require('request');
var querystring = require('querystring');
var _ = require('underscore');
var crypto = require('crypto');
var mongo = require('mongojs');
var bodyParser = require('body-parser');
var app = express();

app.use( bodyParser.json() );       // to support JSON-encoded bodies
app.use(bodyParser.urlencoded({     // to support URL-encoded bodies
    extended: true
}));



// HTML initialization
app.use(express.static(process.cwd() + '/public'));
app.set('views', __dirname + '/www');
app.engine('html', require('ejs').renderFile);

var PORT_NUM = 3001;


var db = mongo('TextDB', ['accounts', 'keys']);
db.keys.drop();

// Web console
app.get("/console", function(req, res) {
    async.waterfall([
        function(callback) {
            request.get("http://localhost/list/", function(error, response, body){
                var urls = _.map(body.split("\n"), function(val){return querystring.escape(val)});
                console.log(body.split("\n"));
                callback(null, {"availableDocs": body.split("\n"), urls: urls});
            });
        }
    ], function(err, result) {
        res.render("index.html", result);
    });

});

// List API for drivers
app.get("/list", function(req, res) {
    async.waterfall([
        function(callback) {
            request.get("http://localhost/list/", function(error, response, body){
                console.log(body.split("\n"));
                callback(null, {"availableDocs": body.split("\n")});
            });
        }
    ], function(err, result) {
        res.type('application/json');
        res.send(result);
    });

});


app.get("/get/:name/:start/:limit", function(req, res) {
    var name = decodeURI(req.params.name);
    var key = req.query.key;
    var start = decodeURI(req.params.start) ? decodeURI(req.params.start) : ;
    var limit = decodeURI(req.params.limit);
    // verify key before serving request
    console.log("(new API) Getting: " + name);
    var url = "http://localhost/get/" + name + "/" + start + "/" + limit;
    request.get(url, function(error, response, body) {
        res.type('text/plain');
        console.log(body);
        res.send(encodeURI(body));
    });
});


app.get("/get/:name/sentence/:start", function(req, res) {
    var name = decodeURI(req.params.name);
    var start = decodeURI(req.params.start);
    console.log("Getting sentence from doc: " + name);
    var url = "http://localhost/sentence/" + name + "/" + start;
    console.log(url);
    request.get(url, function(error, response, body) {
        res.type('text/plain');
        res.send(body);
    });
});


app.get("/adddoc", function(req, res) {

    var query = req.query;
    var name = query.name;
    var path = query.path;
    var url = "http://localhost/adddoc/"+name+"/"+path;
    console.log(name);
    request.get(url, function(error, response, body) {
        res.send("Added: " +  name);
    });

});


app.post("/add", function(req, res) {
    var name = decodeURI(req.query.name);
    var text = decodeURI(req.query.text);
    console.log("Add: " + name + ", " + text);
    var url = "http://localhost/add";
    request.get(url + "/" + name + "/" + text, function(err, r, body) {
        res.send("Added: " + name);
    });
});

app.post("/remove/:name", function(req, res) {
    var name = decodeURI(req.params.name);
    console.log("Removing: " + name);
    var url = "http://localhost/remove";
    request.get(url + "/" + name, function(err, r, body) {
        console.log(body);
        res.send(body);
    });
});


app.get("/get/:name/sentiment", function(req, res) {
    var name = decodeURI(req.params.name);
    console.log("Getting sentiment for: " + name);
    var url = "http://localhost/sentiment";
    request.get(url + "/" + name, function(err, r, body){
        res.send(body);
    });
});


app.get("/get/:name/size", function(req, res){
    var name = decodeURI(req.params.name);
    var url = "http://localhost/size";
    request.get(url + "/" + name, function(err, r, body) {
        res.send(body);
    });
});

app.get("/auth", function(req, res) {
    var username = req.query.username;
    var passhash = req.query.password;
    var sha256 = crypto.createHash('sha256');
    db.accounts.find({'username': username}, function(err, accts) {
        if (accts.length == 0) {
            // invalid username, return nothing
        } else {
            var acct = accts[0];
            var u = acct.username;
            var p = acct.passhash;
            if (p == passhash) {
                sha256 = crypto.createHash('sha256');
                sha256.update(p);
                sha256.update((new Date().getTime()).toString());
                var key = sha256.digest('hex');
                db.keys.insert({'key':key}, function(err, u) {
                    res.type('text/plain');
                    res.send(key);
                });
            }
        }
    });
});

app.post("/auth/create", function(req, res) {
    var username = req.body.username;
    var password = req.body.password;
    console.log("creating account with credentials: " +username + " " + password);

    var sha256 = crypto.createHash('sha256');
    sha256.update(password);
    var passhash = sha256.digest('hex');
    db.accounts.insert({'username': username, 'passhash': passhash}, function(err, doc) {
        // user account creation successful
        res.send();
    });
});
app.get("/auth/create", function(req, res) {
    res.render("create.html");
});

app.get("/", function(req, res) {
    res.redirect("/console");
});


var server = app.listen(PORT_NUM);
console.log('Listening on port ' + server.address().port);
