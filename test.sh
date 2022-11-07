for SIZE in 1000000 5000000 10000000 50000000 100000000
do
    num="$SIZE 1:"
    res1="`./seqsort "$SIZE"  | tail -1`"
    echo "${num} ${res1}" >> result.txt
    for THREAD in 2 3 5 9 13 16
    do
        num="$SIZE $THREAD:"
        resn="`./psort "$SIZE" "$THREAD" | tail -1`"
        echo "${num} ${resn}" >> result.txt
    done
done