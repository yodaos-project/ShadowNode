var fs = require('fs');

const NODE_FILED_OFFSET_TYPE = 0;
const NODE_FIELD_OFFSET_NAME = 1;
const NODE_FIELD_OFFSET_ID = 2;
const NODE_FIELD_OFFSET_SELF_SIZE = 3;

const EDGE_FIELD_OFFSET_TYPE = 0;
const EDGE_FIELD_OFFSET_NAME = 1;
const EDGE_FIELD_OFFSET_TO_NODE = 2;

const DUMMY_STRING = "<dummy>";
const STRING_BASE_OFFSET = 1; // after dummy string

class SnapshotConverter {
  constructor(jerrySnapshotPath, outputPath) {
    var inputData = fs.readFileSync(jerrySnapshotPath, 'utf8');
    this.jerrySnapshot  = JSON.parse(inputData);
    this.outputPath = outputPath;
    this.stringMap = new Map();
    this.nodeMap = new Map();
    this.edgeCount = 0;
    this.nodesOutput = [];
    this.edgesOutput = [];
  }

  parseStrings() {
    for(var i = 0; i < this.jerrySnapshot.strings.length; i++) {
      var stringItem = this.jerrySnapshot.strings[i];
      this.stringMap[stringItem.id] = i + STRING_BASE_OFFSET;
    }
  }

  parseNodes() {
    var edgesCount = 0;
    for(var i = 0; i < this.jerrySnapshot.nodes.length; i++) {
      var nodeItem = this.jerrySnapshot.nodes[i];

      var nodeType = nodeItem.node[NODE_FILED_OFFSET_TYPE];
      this.nodesOutput.push(nodeType);

      var nodeNameID = nodeItem.node[NODE_FIELD_OFFSET_NAME];
      // TODO why this happend?
      if(this.stringMap[nodeNameID] === undefined) {
        this.stringMap[nodeNameID] = 1 // empty string
      }
      this.nodesOutput.push(this.stringMap[nodeNameID]);

      var nodeID = nodeItem.node[NODE_FIELD_OFFSET_ID];
      this.nodeMap[nodeID] = i;
      this.nodesOutput.push(nodeID);

      var nodeSelfSize = nodeItem.node[NODE_FIELD_OFFSET_SELF_SIZE];
      this.nodesOutput.push(nodeSelfSize);

      var nodeEdgeCount = nodeItem.references.length;
      edgesCount += nodeEdgeCount;
      this.nodesOutput.push(nodeEdgeCount);

      this.nodesOutput.push(0); // trace node ID
    }
    this.edgeCount = edgesCount;
  }

  parseEdges() {
    for(var i = 0; i < this.jerrySnapshot.nodes.length; i++) {
      var nodeItem = this.jerrySnapshot.nodes[i];
      var edges = nodeItem.references;

      for(var j = 0; j < edges.length; j++) {
        var edge = edges[j];
  
        var edgeType = edge[EDGE_FIELD_OFFSET_TYPE];
        this.edgesOutput.push(edgeType);

        var edgeNameID = edge[EDGE_FIELD_OFFSET_NAME];

        // TODO why this happend?
        if(this.stringMap[edgeNameID] === undefined) {
          this.stringMap[edgeNameID] = 1 // empty string
        }
        this.edgesOutput.push(this.stringMap[edgeNameID]);

        var edgeToNodeID = edge[EDGE_FIELD_OFFSET_TO_NODE];
        // v8 node items is a flattern array, so to_node is to_node_id * node_size
        this.edgesOutput.push(this.nodeMap[edgeToNodeID] * 6);
      }
    }
  }

  parse() {
    this.parseStrings();
    this.parseNodes();
    this.parseEdges();
  }

  write() {
    var head = "{\"snapshot\":{\"meta\":{\"node_fields\":[\"type\",\"name\",\"id\",\"self_size\",\"edge_count\",\"trace_node_id\"],\"node_types\":[[\"hidden\",\"array\",\"string\",\"object\",\"code\",\"closure\",\"regexp\",\"number\",\"native\",\"synthetic\",\"concatenated string\",\"sliced string\"],\"string\",\"number\",\"number\",\"number\",\"number\",\"number\"],\"edge_fields\":[\"type\",\"name_or_index\",\"to_node\"],\"edge_types\":[[\"context\",\"element\",\"property\",\"internal\",\"hidden\",\"shortcut\",\"weak\"],\"string_or_number\",\"node\"],\"trace_function_info_fields\":[\"function_id\",\"name\",\"script_name\",\"script_id\",\"line\",\"column\"],\"trace_node_fields\":[\"id\",\"function_info_index\",\"count\",\"size\",\"children\"],\"sample_fields\":[\"timestamp_us\",\"last_assigned_id\"]},\"node_count\":"
               + this.jerrySnapshot.nodes.length
               + ",\"edge_count\":"
               + this.edgeCount
               + ",\"trace_function_count\":0},\n\"nodes\":";
    console.log(head);
    console.log(JSON.stringify(this.nodesOutput));
    console.log(",\n\"edges\":");
    console.log(JSON.stringify(this.edgesOutput));
    console.log(",\n\"trace_function_infos\":[],\n\"trace_tree\":[],\n\"samples\":[],\n\"strings\":[");

    console.log("\"<dummy>\"");
    for(var i = 0; i < this.jerrySnapshot.strings.length; i++) {
      var stringItem = this.jerrySnapshot.strings[i];
      console.log(","+JSON.stringify(stringItem.chars));
    }

    console.log("]}");
  }
};

var args = process.argv.splice(2);
var converter = new SnapshotConverter(args[0], args[1]);
converter.parse();
converter.write()
