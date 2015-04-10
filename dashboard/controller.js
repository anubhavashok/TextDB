/**
 * Created by anubhav on 4/9/15.
 */
var isStarted = true;

var onToggleStart = function()
{
    if (isStarted) {
        $("#togglestart").text("Stop");
        isStarted = false;
    } else {
        $("#togglestart").text("Start");
        isStarted = true;
    }
};

var onNodeClick = function(d) {
    $('#name').editable('option', 'value', d.name);

    $('#host').editable('option', 'value', d.host);

    $('#port').editable('option', 'value', d.port);

    $('#status').text(d.status);
    console.log(d.replicas);
    $('#replicas').editable('option', 'value', d.replicas);

    //.append($( "<div id='object1'> "+d.status+" </div>" ));
};

var onSaveClick = function() {

    $("#savealert").append($('<div class="alert alert-success alert-dismissible" role="alert"><button type="button" class="close" data-dismiss="alert" aria-label="Close"><span aria-hidden="true">&times;</span></button><strong>Saved!</strong></div>'));

};