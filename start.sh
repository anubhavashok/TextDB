#!/bin/bash
# script to run db process and link to nginx
DB_PATH=$1
VERBOSE=$2
TEST=$3
DB_EXEC_PATH="/Users/anubhav/Library/Developer/Xcode/DerivedData/TextDB-enrauoafgcwxovajefbwixuxkdug/Build/Products/Debug/TextDB"
PORT_NUM=8000
CWD=$(pwd)
CONF_FILE=$CWD/"nginx.conf"
#sudo nginx -c $CONF_FILE
spawn-fcgi -p $PORT_NUM -n $DB_EXEC_PATH -d $DB_PATH $VERBOSE&
DB_PID=$!
echo "TextDB running on port: "$PORT_NUM
read -p $'Press any key to quit TextDB\n' -n 1 -s end
kill -15 $DB_PID