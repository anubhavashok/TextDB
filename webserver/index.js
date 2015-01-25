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

var url = "http://localhost";


// HTML initialization
app.use(express.static(process.cwd() + '/public'));
app.set('views', __dirname + '/www');
app.engine('html', require('ejs').renderFile);

var PORT_NUM = 3001;


var db = mongo('TextDB', ['accounts', 'keys']);
db.keys.drop();

// Web console
app.get("/console", function(req, res) {
    var uri = joinSlash(url, "listcollections");

    logEntry(req.method, req.path);

    async.waterfall([
        function(callback) {
            request.get(uri, function(error, response, body){
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
    var uri = joinSlash(url, "listcollections");

    logEntry(req.method, req.path);

    async.waterfall([
        function(callback) {
            request.get(uri, function(error, response, body){
                console.log(body);
                callback(null, body);
            });
        }
    ], function(err, result) {
        res.type('application/json');
        res.send(result);
    });

});

app.get("/list/:collection", function(req, res) {
    var collection = decodeURI(req.params.collection);

    var uri = joinSlash(url, "listdocs", collection);

    logEntry(req.method, req.path);

    async.waterfall([
        function(callback) {
            request.get(uri, function(error, response, body){
                console.log(body);
                callback(null, body);
            });
        }
    ], function(err, result) {
        res.type('application/json');
        res.send(result);
    });

});


app.get("/get/:collection/:name", function(req, res) {
    var collection = decodeURI(req.params.collection);
    var name = decodeURI(req.params.name);
    var key = req.query.key;


    var uri = joinSlash(url, "get", collection, name);

    logEntry(req.method, req.path);

    // verify key before serving request
    request.get(uri, function(error, response, body) {
        res.type('text/plain');
        console.log(body);
        res.send(body);
    });
});

app.get("/get/:collection/", function(req, res) {
    var collection = decodeURI(req.params.collection);
    var key = req.query.key;
    // verify key before serving request

    var uri = joinSlash(url, "listdocs", collection);

    logEntry(req.method, req.path);

    request.get(uri, function(error, response, body) {
        res.type('application/json');
        console.log(body);
        res.send(encodeURI(body));
    });
});


app.get("/get/:collection/:name/sentence/:start", function(req, res) {
    var collection = decodeURI(req.params.collection);
    var name = decodeURI(req.params.name);
    var start = decodeURI(req.params.start);

    var uri = joinSlash(url, "sentence", collection, name, start);

    logEntry(req.method, req.path);

    request.get(uri, function(error, response, body) {
        res.type('text/plain');
        res.send(body);
    });
});

app.post("/add/:collection/:name/:text", function(req, res) {
    var collection = decodeURI(req.params.collection);
    var name = decodeURI(req.params.name);
    var text = decodeURI(req.params.text);

    var uri = joinSlash(url, "add", collection, name, text);

    logEntry(req.method, req.path);

    request.get(uri, function(err, r, body) {
        //
        res.send("True");
    });
});

app.post("/remove/:collection/:name", function(req, res) {
    var collection = decodeURI(req.params.collection);
    var name = decodeURI(req.params.name);

    logEntry(req.method, req.path);

    var uri = joinSlash(url, "remove", collection, name);
    request.get(uri, function(err, r, body) {
        console.log(body);
        res.send(body);
    });
});


app.get("/get/:collection/:name/sentiment", function(req, res) {
    var collection = decodeURI(req.params.collection);
    var name = decodeURI(req.params.name);

    logEntry(req.method, req.path);

    var uri = joinSlash(url, "sentiment", collection, name);

    request.get(uri, function(err, r, body){
        res.send(body);
    });
});


app.get("/get/:collection/:name/size", function(req, res) {
    var collection = decodeURI(req.params.collection);
    var name = decodeURI(req.params.name);

    logEntry(req.method, req.path);

    var uri = joinSlash(url, "size", collection, name);

    request.get(uri, function(err, r, body) {
        res.send(body);
    });
});

app.post("/drop/:collection", function(req, res) {
    var collection = decodeURI(req.params.collection);

    logEntry(req.method, req.path);

    var uri = joinSlash(url, "drop", collection);

    request.get(uri, function(err, r, body){
        res.send("True");
    });
});

app.post("/create/:collection/:encoding", function(req, res) {
    var collection = decodeURI(req.params.collection);
    var encoding = decodeURI(req.params.encoding);

    logEntry(req.method, req.path);

    var uri = joinSlash(url, "create", collection, encoding);
    request.get(uri, function(err, r, body){
        res.send("True");
    });
});

app.get("/disk_size/:collection", function(req, res) {
    var collection = decodeURI(req.params.collection);

    logEntry(req.method, req.path);

    var uri = joinSlash(url, "collectionsize", collection);

    request.get(uri, function(err, r, body) {
        res.send(body);
    });
});

app.get("/auth", function(req, res) {
    var username = req.query.username;
    var passhash = req.query.password;
    var sha256 = crypto.createHash('sha256');

    logEntry(req.method, req.path);

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

    logEntry(req.method, req.path);

    var sha256 = crypto.createHash('sha256');
    sha256.update(password);
    var passhash = sha256.digest('hex');
    db.accounts.insert({'username': username, 'passhash': passhash}, function(err, doc) {
        // user account creation successful
        res.send();
    });
});
app.get("/auth/create", function(req, res) {

    logEntry(req.method, req.path);

    res.render("create.html");
});

app.get("/", function(req, res) {

    logEntry(req.method, req.path);

    res.redirect("/console");
});

// function that takes in a list of arguments and returns a string of then joined by slashes
function joinSlash()
{
    var arr = [];
    var args = arguments;
    Object.keys(args).forEach(function(k) {
        arr.push(args[k]);
    });
    return arr.join('/') + '/';
}

function logEntry(method, path)
{
    //console.log("(Node): " + arguments[0] + " " + joinSlash.apply(this, arguments.slice(1)));
    console.log("(Node): " + method + " " + path);
}

var server = app.listen(PORT_NUM);
console.log('Listening on port ' + server.address().port);
