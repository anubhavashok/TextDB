/**
 * Created by anubhav on 4/18/15.
 */
var express = require('express');
var ssh = require('simple-ssh');
var fs = require('fs');
var login = require('./login.js');
var md5 = require('./md5.js');
var path = require('path');
var ejs = require('ejs');
var sjcl = require('sjcl');
var sequest = require('sequest');
var process = require('process');

var bodyParser = require('body-parser');

var app = express();

app.use(express.static(__dirname));

// create application/json parser
app.use(bodyParser.urlencoded({"extended": false}));

app.set("views", __dirname);
app.engine('.html', ejs.__express);
app.set('view engine', 'html');


var loggedIn = false;
var currentAccount;

var accounts = [];
var hosts = [];

app.get('/dashboard', function (req, res) {
    if (!loggedIn) {
        res.redirect('/login');
    }
    res.render("dashboard");
});

app.get('/hosts', function (req, res) {
    if (!loggedIn) {
        res.redirect('/login');
    }
    res.render("hosts", {"hosts": hosts});
});

app.post('/hosts', function(req, res) {
    // add new host
    var host = {};
    host["name"] = req.body.name;
    host["address"] = req.body.address;
    host["port"] = req.body.port;
    host["username"] = req.body.username;
    host["password"] = sjcl.encrypt(currentAccount["password"], req.body.password);
    hosts.push(host);
    fs.writeFile('hosts.json', JSON.stringify(hosts), function (err) {
        if (err) throw err;
    });
    res.redirect('/hosts');
});

app.post('/hosts/:id/remove', function(req, res) {
    var id = parseInt(req.params.id);
    console.log("Removing host with id: " + id);
    hosts.splice(id, 1);

    fs.writeFile('hosts.json', JSON.stringify(hosts), function (err) {
        if (err) throw err;
    });
    res.redirect('/hosts');
});

app.post('/hosts/:id/start', function(req, res) {
    var id = parseInt(req.params.id);
    var host = hosts[id];
    var hostAddress =  hosts[id]["address"];
    var username = hosts[id]["username"];
    var password = sjcl.decrypt(currentAccount["password"], hosts[id]["password"]);
    console.log("sshing to '" + hostAddress + "'");
    console.log("username, '" + hosts[id]["username"] + "'");
    console.log("password, '" + sjcl.decrypt(currentAccount["password"], hosts[id]["password"]) + "'");

    var Client = require('ssh2').Client;

    var conn = new Client();
    conn.on('ready', function() {
        console.log('Client :: ready');
        // create command to run using saved parameters in hosts[id]
        var command = "";
        //command += "-p " + hosts[id]["port"];
        command += "-c " + hosts[id]["config"];


        conn.exec('TextDBe ' + command, function(err, stream) {
            if (err) throw err;
            stream.on('close', function(code, signal) {
                // console.log('Stream :: close :: code: ' + code + ', signal: ' + signal);
                conn.end();
            }).on('data', function(data) {
                console.log('STDOUT: ' + data);
            }).stderr.on('data', function(data) {
                    console.log('STDERR: ' + data);
                });
        });
    }).connect({
        host: hostAddress,
        port: 22,
        username: username,
        password: password
    });


    res.redirect('/hosts');
});

app.get('/logout', function(req, res) {
    delete currentAccount;
    delete hosts;
    loggedIn = false;
    res.redirect("/login");
});

var needAdmin = true;
app.get('/login', function (req, res) {
    res.render("login", {"needAdmin": needAdmin, "err": false});
    //res.sendFile(path.join(__dirname, 'login.html'));
});

app.post('/login', function(req, res){
    var username = req.body.username;
    var password = req.body.password;
    var admin = req.body.admin;
    for (var i = 0; i < accounts.length; i++) {
        var account = accounts[i];
        if (account.username == username) {
            var passwordHash = md5.hex_md5(password);
            if (passwordHash == account.passwordHash) {
                loggedIn = true;
                currentAccount = {"username":username, "password":password, "admin":admin};
                // TODO: partition hosts according to user and read only after logging in
                fs.readFile("hosts.json", 'utf-8', function(err, data){
                    hosts = JSON.parse(data);
                    console.log(hosts);
                });

                res.redirect("/dashboard");
                return;
            }
        }
    }
    res.render("login", {"err": "Login failed", "needAdmin": needAdmin});
});

app.get('/createaccount', function(req, res) {

    res.render('createaccount');
});

app.post('/createaccount', function(req, res) {
    var username = req.body.username;
    var password = req.body.password;
    var admin = req.body.admin == "on";
    var account = {};
    account["username"] = username;
    account["passwordHash"] = md5.hex_md5(password);
    account["admin"] = admin;
    for (var i = 0; i < accounts.length; i++) {
        if (accounts[i].username == username) {
            res.render("createaccount", {"err": true});
            return;
        }
    }

    accounts.push(account);
    fs.writeFile('accounts.json', JSON.stringify(accounts), function (err) {
        if (err) throw err;
    });
    res.redirect("/login");
});

var server = app.listen(3000, function () {

    var host = server.address().address;
    var port = server.address().port;

    console.log('Example app listening at http://%s:%s', host, port);
    fs.readFile("accounts.json", 'utf-8', function(err, data){
        console.log(data);
        accounts = JSON.parse(data);
        for (var i = 0; i < accounts.length; i++) {
            console.log(accounts[i]);
            if (accounts[i].admin == true) {
                needAdmin = false;
            }
        }
    });

});
