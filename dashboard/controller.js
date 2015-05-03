/**
 * Created by anubhav on 4/9/15.
 */
var isStarted = true;
var currentNode;

var onToggleStart = function()
{
    var button = $("#togglestart");
    if(currentNode.status == "OK") {
        // try to stop node
        console.log("Stopping " + currentNode.name);
        $.post(currentNode.host+":"+currentNode.port+"/"+"stop", function() {

        });
    } else {
        // try to start node
        console.log("Starting " + currentNode.name);
    }
};

var onNodeClick = function(d) {
    currentNode = d;
    $('#name').editable('option', 'value', d.name);

    $('#host').text(d.host);

    $('#port').editable('option', 'value', d.port);

    $('#status').text(d.status);
    console.log(d);
    $('#replicas').editable('option', 'value', d.replicas);

    if (currentNode.status == "OK") {
        $("#togglestart").text("Stop");
    } else {
        $("#togglestart").text("Start");
    }
    //.append($( "<div id='object1'> "+d.status+" </div>" ));
};

var onSaveClick = function() {
    // Update links
    var newLinks = $("#replicas").editable('getValue')["replicas"];
    console.log(newLinks);
    newLinks = newLinks.map(function(l) {
        return parseInt(l);
    });
    updateLinks(data.nodes[currentNode.index].replicas, newLinks);

    // Update name
    data.nodes[currentNode.index].name = $("#name");
    $("#savealert").append($('<div id="dismissablesavealert" class="alert alert-success alert-dismissible" role="alert"><button type="button" class="close" data-dismiss="alert" aria-label="Close"><span aria-hidden="true">&times;</span></button><strong>Saved!</strong></div>'));
    $("#dismissablesavealert").fadeOut(2500);
    // if delete node,
    if ($("#deleteNode").is(':checked')) {
        // stop node
        // remove node
        deleteNode(currentNode.index);
    }
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
    nodeNames.push({id:data.nodes.length-1, text:name});
    $('#myModal').modal('hide');
    update(data);
    $("#addNodeForm")[0].reset();
};


var addLinks = function(l){
    l.value.forEach(function(value){
        var index = parseInt(value[0]);
        var target = data.nodes[index];
        var source = data.nodes[currentNode.index];
        data.nodes[currentNode.index].replicas.push(index);
        data.nodes[index].replicas.push(currentNode.index);
        console.log();
        console.log(target);
        console.log(source);
        data.links.push({source: source, target: target});
    });
    update(data);
};

var deleteNode = function(id)
{
    // stop node
    data.nodes.splice(id, 1);
    for (var i = 0; i < data.links.length; i++) {
        var l = data.links[i];
        if ((l.source.index == id) || (l.target.index == id)) {
            data.links.splice(i, 1);
            i--;
        }
    }
    nodeNames.splice(id, 1);
    update(data);
    resetOptionsTab();
};

var updateLinks = function(o, n)
{
    console.log("updating links");
    console.log(o);
    console.log(n);
    // o - old links
    // n - new links
    // old - intersection of sets = to be removed
    // new - intersection of sets = to be added
    var intersection = _.intersection(o, n);

    var tbremoved = _.difference(o, intersection);
    var tbadded = _.difference(n, intersection);
    console.log(tbremoved);
    console.log(tbadded);
    data.nodes[currentNode.index].replicas = _.difference(data.nodes[currentNode.index].replicas, tbremoved);
    data.nodes[currentNode.index].replicas.concat(tbadded);
    for (var i = 0; i < data.links.length; i++) {
        tbremoved.forEach(function(r){
            if ((data.links[i].source.index == currentNode.index) && (data.links[i].target.index == r) ||
                (data.links[i].target.index == currentNode.index) && (data.links[i].source.index == r)
                ) {
                data.links.splice(i, 1);
            }
        });
    }
    update(data);
};

var resetOptionsTab = function()
{
    $("#replicas").editable('setValue', "-");
    $("#host").text("-");
    $("#status").text("-");
    $("#name").editable('setValue', "-");
    $("#name").editable('disable');
    $("#deleteNode").removeAttr('checked');

};
