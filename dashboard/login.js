/**
 * Created by anubhav on 4/18/15.
 */

var accounts = [];

var signIn = function()
{
    // load accounts.json, check if account is present and valid
    // if so, redirect to dashboard.html

};

function errorHandler(e) {
    var msg = '';
    console.log(e.name);
    switch (e.code) {
        case FileError.QUOTA_EXCEEDED_ERR:
            msg = 'QUOTA_EXCEEDED_ERR';
            break;
        case FileError.NOT_FOUND_ERR:
            msg = 'NOT_FOUND_ERR';
            break;
        case FileError.SECURITY_ERR:
            msg = 'SECURITY_ERR';
            break;
        case FileError.INVALID_MODIFICATION_ERR:
            msg = 'INVALID_MODIFICATION_ERR';
            break;
        case FileError.INVALID_STATE_ERR:
            msg = 'INVALID_STATE_ERR';
            break;
        default:
            msg = 'Unknown Error';
            break;
    }

    console.log('Error: ' + msg);
}

var newAccount = function()
{
    var username = $("#username");
    var password = $("#password");
    var account = {};
    account["type"] = $("#admin").prop("checked")? "admin" : "nonadmin";
    account["username"] = username;
    account["passwordHash"] = hex_md5(password);
    accounts.push(account);
    // send request to server
};



