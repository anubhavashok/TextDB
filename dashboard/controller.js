/**
 * Created by anubhav on 4/9/15.
 */
var isStarted = true;
var currentNode;

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
    currentNode = d;
    $('#name').editable('option', 'value', d.name);

    $('#host').editable('option', 'value', d.host);

    $('#port').editable('option', 'value', d.port);

    $('#status').text(d.status);
    console.log(d);
    $('#replicas').editable('option', 'value', d.replicas);

    //.append($( "<div id='object1'> "+d.status+" </div>" ));
};

var onSaveClick = function() {

    $("#savealert").append($('<div class="alert alert-success alert-dismissible" role="alert"><button type="button" class="close" data-dismiss="alert" aria-label="Close"><span aria-hidden="true">&times;</span></button><strong>Saved!</strong></div>'));

};

var addNode = function() {
    // ensure node is not a duplicate
    // remember to save in data.json
    var name = $("#anname").val(),
        host = $("#anhost").val(),
        port = $("#anport").val();

    var sum_x = 0, sum_y = 0;
    data.nodes.forEach(function(n) {

        sum_x += n.x;
        sum_y += n.y;

    });

    var center_x = sum_x/data.nodes.length,
        center_y = sum_y/data.nodes.length;

    var node = {x: center_x, y: center_y, name: name, host: host, port: port, status: status, replicas: []};
    data.nodes.push(node);
    $('#myModal').modal('hide');
    update(data);
};


var addLinks = function(l, t){
    l.value.forEach(function(value){
        var index = parseInt(value[0]);
        var target = data.nodes[index];
        var source = data.nodes[currentNode.index];
        data.nodes[currentNode.index].replicas.push(index);
        data.nodes[index].replicas.push(currentNode.index);
        console.log(target);
        console.log(source);
        data.links.push({source: source, target: target});
    });
    update(data);
};