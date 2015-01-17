/*
node:0 x,y,w,h,name, xform_name
workspace:0 nodes_list
workspace_inputs:0 input_node input_name input_node2 input_name2
workspace_outputs:0 output_node output_name output_node2 output_name2
connections:0 input_name (node, out_name)
constants:0 input_name constant

Use out_id instead of out_name for easy renaming of outputs

Workspaces are just clonable parent nodes with xform_name: ws_[id]
*/
package main

import (
    "net/http"
    "github.com/gorilla/websocket"
    "log"
    "github.com/fzzy/radix/redis"
    "strings"
    "fmt"
    "time"
    "strconv"
)

var upgrader = websocket.Upgrader {
    ReadBufferSize:     1024,
    WriteBufferSize:    1024,
}

func errHndlr(err error) {
    if err != nil {
        fmt.Println("error:", err)
    }
}

// Every input only has one output but outputs can be fed to many different inputs
// output -> input mapping is stored in each connection
func UpdateConnection(args []string, rc *redis.Client) {
    nodeID      := args[0]
    input_name  := args[1]
    out_node    := args[2]
    output_name := args[3]
    r := rc.Cmd("HSET", "connections:" + nodeID, input_name, out_node + " " + output_name)
    errHndlr(r.Err)
}

func DeleteConnectionsToNode(nodeID string, rc *redis.Client, conn *websocket.Conn) {
    // There are at most nextid nodes in the system
    // If this becomes costly, create a reverse mapping nodes->connections for faster search
    nextid, err := rc.Cmd("GET", "nextNodeID").Int64()
    errHndlr(err)

    var i int64
    for i = 1; i <= nextid; i++ {
        hash_name := "connections:" + strconv.FormatInt(i, 10)
        connections, err := rc.Cmd("HGETALL", hash_name).Hash()
        errHndlr(err)
        for input_name, connection := range connections {
            splitted := strings.SplitN(connection, " ", 2)    
            input_node := splitted[0] 
            fmt.Println("Connection for " + input_name + " ->" + input_node + " when deleting " + nodeID)
            if input_node == nodeID {
                reply := "deleteconn " + strconv.FormatInt(i, 10) + " " + input_name
                rc.Cmd("HDEL", hash_name, input_name)
                conn.WriteMessage(websocket.TextMessage, []byte(reply))
            }
        }
    }
}

func DeleteNode(args []string, rc *redis.Client) string {
    nodeID      := args[0]
    r := rc.Cmd("DEL", "nodes:" + nodeID)
    errHndlr(r.Err)
    r = rc.Cmd("DEL", "connections:" + nodeID)
    errHndlr(r.Err)

    return "deletenode " + nodeID
}
func DeleteConnection(args []string, rc *redis.Client) {
    nodeID      := args[0]
    input_name  := args[1]
    r := rc.Cmd("HDEL", "connections:" + nodeID, input_name)
    errHndlr(r.Err)
}

// expires in ms
func Lock(key, value, expires string, rc *redis.Client) bool {
    r := rc.Cmd("SET", key, value, "NX")
    errHndlr(r.Err)
    if r.Type == redis.NilReply {
        return false
    } else {
        r = rc.Cmd("EXPIRE", key, expires)
        return true
    }
}

func Unlock(key string, rc *redis.Client) {
    r := rc.Cmd("DEL", key)
    errHndlr(r.Err)
}

func LockNode(args []string, rc *redis.Client) string {
    node := args[0]
    username := args[1]
    expires := args[2]
    success := Lock("locknode:" + node, username, expires, rc)

    if success {
        return "Lock successful for node " + node + "!"
    } else {
        return "Lock failed for node " + node + "!"
    }
}

func UnlockNode(args []string, rc *redis.Client) string {
    node := args[0]
    username := args[1]

    r := rc.Cmd("GET", "locknode:" + node)
    lockingUser, err := r.Str()
    errHndlr(err)
    if lockingUser != username {
        return "Unlock failed! Current lock owner " + lockingUser + " != " + username
    }
    Unlock("locknode:" + node, rc)
    return "Unlock succesful!"
}

func GetNodeOwner(node string, rc *redis.Client) string {
    r := rc.Cmd("GET", "locknode:" + node)
    lockingUser, err := r.Str()
    errHndlr(err)
    return lockingUser
}

func UpdateConstant(args []string, rc *redis.Client) {
    nodeID      := args[0]
    input_name  := args[1]
    constant    := args[2]
    r := rc.Cmd("HSET", "constants:" + nodeID, input_name, constant)
    errHndlr(r.Err)
}

func CreateNode(args []string, rc *redis.Client) string {
    node := map[string]string {
        "x" : args[0],
        "y" : args[1],
        "w" : args[2],
        "h" : args[3],
        "xform" : args[4],
        "name" : args[5],
    }
    fmt.Println(strings.Join(args, " - "))
    nextNodeID, err := rc.Cmd("INCR", "nextNodeID").Int64()
    errHndlr(err)
    r := rc.Cmd("HMSET", "nodes:" + strconv.FormatInt(nextNodeID, 10), node)
    errHndlr(r.Err)

    return "createnode " + strconv.FormatInt(nextNodeID, 10) + " " + strings.Join(args, " ")
}

func UpdateNodePos(args []string, rc *redis.Client) {
    nodeID := args[0]
    x      := args[1]
    y      := args[2]
    r := rc.Cmd("HMSET", "nodes:" + nodeID, "x", x, "y", y)
    errHndlr(r.Err)
}

func UpdateNodeSize(args []string, rc *redis.Client) {
    nodeID := args[0]
    w      := args[1]
    h      := args[2]
    r := rc.Cmd("HMSET", "nodes:" + nodeID, "w", w, "h", h)
    errHndlr(r.Err)
}

func CreateConstMsg(constants map[string]string) (string){
    var msg string = ""
    // Format for the message is input_name const_len const input_name2 const_len2 const2
    for input_name, constant := range constants {
        msg += input_name + " " + strconv.FormatInt(int64(len(constant)), 10) + " " + constant + " "
    }
    return msg
}
func CreateConnMsg(conn map[string]string) (string){
    var msg string = ""
    // Format for the message is input_name out_node output_name input_name2 out_node2 output_name2
    for input_name, connection := range conn {
        msg += input_name + " " + connection + " "
    }
    return msg
}
func CreateNodeMsg(node map[string]string) (bool, string){
    fields := [...]string{"x", "y", "w", "h", "xform", "name"}
    var msg string = ""
    for _, field := range fields {
        if val, ok := node[field]; ok { msg += val + " "} else { return false, "" }
    }
    return true, msg
}

// Push deleted node ids to a new list for reuse later...
func GetWorkspace(args []string, rc *redis.Client, conn *websocket.Conn) {
    nextid, err := rc.Cmd("GET", "nextNodeID").Int64()
    errHndlr(err)

    var i int64
    for i = 1; i <= nextid; i++ {
        node, err := rc.Cmd("HGETALL", "nodes:" + strconv.FormatInt(i, 10)).Hash()
        errHndlr(err)
        succ, msg := CreateNodeMsg(node)
        if succ {
            reply := "createnode " + strconv.FormatInt(i, 10) + " " + msg
            fmt.Println("Replied with>>> " + reply)
            conn.WriteMessage(websocket.TextMessage, []byte(reply))
        }
        connections, err := rc.Cmd("HGETALL", "connections:" + strconv.FormatInt(i, 10)).Hash()
        msg = CreateConnMsg(connections)
        if msg != "" {
            reply := "connections " + strconv.FormatInt(i, 10) + " " + msg
            fmt.Println("Replied with>>> " + reply)
            conn.WriteMessage(websocket.TextMessage, []byte(reply))
        }
        constants, err := rc.Cmd("HGETALL", "constants:" + strconv.FormatInt(i, 10)).Hash()
        msg = CreateConstMsg(constants)
        if msg != "" {
            reply := "consts " + strconv.FormatInt(i, 10) + " " + msg
            fmt.Println("Replied with>>> " + reply)
            conn.WriteMessage(websocket.TextMessage, []byte(reply))
        }
    }
}
func UpdateNodeOperator(args []string, rc *redis.Client) {
    nodeID := args[0]
    xform  := args[1]
    name   := args[2]
    r := rc.Cmd("HMSET", "nodes:" + nodeID, "xform", xform, "name", name)
    errHndlr(r.Err)
}

func CmdHandler(cmd string, rc *redis.Client, conn *websocket.Conn) string {
    args := strings.Split(cmd, " ")
    switch args[0] {
        case "Workspace":
            GetWorkspace(args[1:], rc, conn)
        case "CreateNode":
            return CreateNode(args[1:], rc)
        case "LockNode":
            return LockNode(args[1:], rc)
        case "UnlockNode":
            return UnlockNode(args[1:], rc)
        case "UpdateNodeOp":
            UpdateNodeOperator(args[1:], rc)
        case "UpdateNodePos":
            UpdateNodePos(args[1:], rc)
        case "UpdateNodeSize":
            UpdateNodeSize(args[1:], rc)
        case "UpdateConst":
            UpdateConstant(args[1:], rc)
        case "UpdateConn":
            UpdateConnection(args[1:], rc)
        case "DeleteConn":
            DeleteConnection(args[1:], rc)
        case "DeleteNode":
            // Delete connections to this node too, since they're pointing to a dead node now
            DeleteConnectionsToNode(args[1], rc, conn)
            return DeleteNode(args[1:], rc)
    }
    return ""
}

func wsHandler(w http.ResponseWriter, req *http.Request) {
    var rc *redis.Client
    conn, err := upgrader.Upgrade(w, req, nil)
    if err != nil {
        log.Println("Error upgrading connection", err)
        return
    }
    rc, err = redis.DialTimeout("tcp", "127.0.0.1:6379", time.Duration(10)*time.Second)
    errHndlr(err)
    defer rc.Close()

    for {
        _, p, err := conn.ReadMessage()
        if err != nil {
            errHndlr(err)
            return
        }

        fmt.Println(string(p))
        reply := CmdHandler(string(p), rc, conn)
        if reply != "" {
            conn.WriteMessage(websocket.TextMessage, []byte(reply))
        }
    }
}

func main() {
    http.HandleFunc("/luminos", wsHandler)
    http.ListenAndServe(":8126", nil)
}
