/**
 * Created by anubhav on 4/9/15.
 */
var width = 600,
    height = 500;

var svg = d3.select("#fgraph").append("svg")
    .attr("width", width)
    .attr("height", height);

var force = d3.layout.force()
    .gravity(.05)
    .distance(100)
    .charge(-300)
    .size([width, height]);

var nodes = {};
var links = {};
var nodeNames = [];
var data;
var node;
var link;

d3.json("data.json", function(error, json) {
    data = json;
    console.log(json);
    force
        .nodes(json.nodes)
        .links(json.links)
        .start();

    link = svg.selectAll(".link");
    link = link.data(json.links);
    link
        .enter().append("line")
        .attr("class", "link");

    for (var i=0; i < json.links.length; i++) {
        var l = json.links[i];
        if (!(l.source.index in links)) {
            links[l.source.index] = [];
        }
        links[l.source.index].push(l.target.index);

        if(!(l.target.index in links)) {
            links[l.target.index] = [];
        }
        links[l.target.index].push(l.source.index);
    }
    console.log(links);

    node = svg.selectAll("g.node");
    node = node.data(json.nodes);
    node.enter()
        .append("g")
        .attr("class", "node")
        .call(force.drag);
    for (var i=0; i < node[0].length; i++) {
        console.log(i);
        var n = node[0][i].__data__.name;
        nodes[n] = node[0][i];
        nodeNames.push({"id":i, "text":n});
        node[0][i].__data__.replicas = [];
        for (var j=0; j < links[node[0][i].__data__.index].length; j++) {
            node[0][i].__data__.replicas.push(links[node[0][i].__data__.index][j]);
        }
        //node[0][i].replicas = [];
    }
    console.log(node);
    force.on("tick", function() {
        link.attr("x1", function(d) { return d.source.x; })
            .attr("y1", function(d) { return d.source.y; })
            .attr("x2", function(d) { return d.target.x; })
            .attr("y2", function(d) { return d.target.y; });

        node.attr("transform", function(d) { return "translate(" + d.x + "," + d.y + ")"; });
    });
    update(data);
});

$(document).ready(function(){

    //$('#name').editable();


});


function update(json) {
    link.remove();
    link = svg.selectAll(".link")
        .data(json.links);
    link
        .enter()
        .insert("line", ".node")
        .attr("class", "link");

    node.remove();
    node = svg.selectAll("g.node")
        .data(json.nodes);

    force
        .nodes(json.nodes)
        .links(json.links)
        .start();
    node
        .enter().append("g")
        .attr("class", "node");
        node.call(force.drag);

    node.insert("svg:image")
        .attr("xlink:href", "http://www.rw-designer.com/icon-image/7506-256x256x32.png")
        .attr("x", -40)
        .attr("y", -8)
        .attr("width", 80)
        .attr("height", 80);

    node.insert("text")
        .attr("dx", 12)
        .attr("dy", 85)
        .text(function (d) {
            return d.name
        });
    node.insert("image")
        .attr("xlink:href", function(d){
            if(d.status == "OK") {
                return "http://files.softicons.com/download/toolbar-icons/16x16-free-application-icons-by-aha-soft/png/16x16/Yes.png";
            } else {
                return "http://files.softicons.com/download/toolbar-icons/max-mini-icons-by-ashung/ico/crossout.ico";
            }
        })
        .attr("x", 11)
        .attr("y", -4)
        .attr("width", 20)
        .attr("height", 20);
    node.on("click", onNodeClick);

    $('#replicas').editable({
        inputclass: 'input-large',
        url: addLinks,
        source: nodeNames,
        select2: {
            width: 200,
            placeholder: 'Select node',
            allowClear: true,
            multiple: true
        }
    });
}

// ping each replica every 2.5 seconds to get status
function ping()
{
    data.nodes.forEach(function(n){
        $.get(n.host+"/ping", function(data) {
            console.log(data);
            n.status = "OK";
        })
        .fail(function(data) {
            n.status = "Down";
        });
        setTimeout(ping, 25000);
    });

}


//setTimeout(ping, 25000);