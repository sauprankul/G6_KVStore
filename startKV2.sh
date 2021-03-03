if [[ $# -eq 0 ]]
    then
        echo "Need argument n (int)"
        return
fi

counter=0
while [[ counter -lt $1 ]]
do
    port=50000
    port=$(($port + $counter))
    java -jar KVServer2_1.jar $port $counter $1 none &
    counter=$(($counter + 1))
done

echo "Script done"
