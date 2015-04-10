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

d3.json("data.json", function(error, json) {
    force
        .nodes(json.nodes)
        .links(json.links)
        .start();

    var link = svg.selectAll(".link")
        .data(json.links)
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

    var node = svg.selectAll(".node")
        .data(json.nodes)
        .enter().append("g")
        .attr("class", "node")
        .call(force.drag);

    for (var i=0; i < node[0].length; i++) {
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

    $('#replicas').editable({
        inputclass: 'input-large',
        source: nodeNames,
        select2: {
            width: 200,
            placeholder: 'Select node',
            allowClear: true,
            multiple: true
        }
    });
    node.append("image")
        .attr("xlink:href", "https://cdn3.iconfinder.com/data/icons/programming/100/database_1-128.png")
        .attr("x", -40)
        .attr("y", -8)
        .attr("width", 80)
        .attr("height", 80);

    node.append("text")
        .attr("dx", 12)
        .attr("dy", 85)
        .text(function(d) { return d.name });
    node.on("click", onNodeClick);

    force.on("tick", function() {
        link.attr("x1", function(d) { return d.source.x; })
            .attr("y1", function(d) { return d.source.y; })
            .attr("x2", function(d) { return d.target.x; })
            .attr("y2", function(d) { return d.target.y; });

        node.attr("transform", function(d) { return "translate(" + d.x + "," + d.y + ")"; });
    });
});

$(document).ready(function(){

    //$('#name').editable();


});