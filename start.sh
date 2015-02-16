#!/bin/bash
# script to run db process and link to nginx
CWD=$(pwd)
DB_PATH=$CWD/"store.bindb"
[[-n "$1"]] && DB_PATH=$1
VERBOSE=$2
TEST=$3
DATA_PATH=$CWD/"data"
DB_EXEC_PATH="/Users/anubhav/Library/Developer/Xcode/DerivedData/TextDB-enrauoafgcwxovajefbwixuxkdug/Build/Products/Debug/TextDB"
#PORT_NUM=8000

CONF_FILE=$CWD/"nginx.conf"

$DB_EXEC_PATH -d $DB_PATH -a $DATA_PATH $VERBOSE &
DB_PID=$!
#echo "TextDB running on port: "$PORT_NUM
read -p $'Press any key to quit TextDB\n' -n 1 -s end
kill -15 $DB_PID
