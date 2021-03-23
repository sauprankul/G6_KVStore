if [[ $# -eq 0 ]]
    then
        echo "Need argument for filename"
        return
fi

counter=0
lines=$(wc -l < "$1")
while [[ counter -le lines ]]
do
    echo $counter
    java -jar KVServer3_*.jar $1 $counter &
    counter=$(($counter + 1))
done

echo "Script done"
