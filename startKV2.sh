if [[ $# -eq 0 ]]
    then
        echo "Need argument for filename"
        return
fi

counter=0
lines=$(wc -l < "$1")
echo "$lines" "lines detected. (please check that there is a newline at the end of the config file"
while [[ counter -lt lines ]]
do
    echo $counter
    java -jar KVServer2_*.jar $1 $counter &
    counter=$(($counter + 1))
done

echo "Script done"
