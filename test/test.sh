NWORDS=100
NFILES=10

CWD=$(pwd)

rm "$CWD/store.bindb"
rm "$CWD/store.bindb.kvp"
rm "$CWD/store.text"

DB_PATH="$CWD/store.bindb"
VERBOSE="-v"
DB_EXEC_PATH="/Users/anubhav/Library/Developer/Xcode/DerivedData/TextDB-enrauoafgcwxovajefbwixuxkdug/Build/Products/Debug/TextDB"
PORT_NUM=8000
spawn-fcgi -p $PORT_NUM -n $DB_EXEC_PATH -d $DB_PATH &

DB_PID=$!
echo "TextDB running, PID (If anything goes wrong, manually stop TextDB using kill -15 <PID>): $DB_PID"

i=0
testfilessize=0
for f in $CWD/files/*
    do
        echo "Adding file $f"
        A=$(printf "\\$(printf '%03o' $((97+$i)))")
        curl -i "localhost?adddoc&$A&$f" &> /dev/null
        curl -i "localhost?get&$A" &> /dev/null
        testfilessize=$(( testfilessize + $(wc -c "$f" | awk '{print $1}')))
        i=$((i+1))
    done

kill -15 $DB_PID

# Delay output of matrix while waiting for files to be written
writedelay=$(echo "2.5 * $testfilessize/1048576" | bc -l)

printf "Sleeping for %.*f seconds while TextDB is being persisted\n" 2 $writedelay

while [ ! -f "$CWD/store.bindb.kvp" ]
    do
        sleep 0.1
    done
sleep $writedelay

sizekvp=$(wc -c "$CWD/store.bindb.kvp" | awk '{print $1}')
sizeidx=$(wc -c "$CWD/store.bindb" | awk '{print $1}')
sizeraw=$(wc -c "$CWD/store.text" | awk '{print $1}')
sizetextdb=$((sizeidx + sizekvp))
compression=$(echo "100 - 100 * $sizetextdb/$sizeraw" | bc -l)

echo "Uncompressed file size: $sizeraw bytes"
echo "TextDB size: $sizetextdb bytes"
printf "COMPRESSION RATIO: %.*f%% \n" 2 $compression