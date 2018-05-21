min_node_size_list="5
10
15
20
25"

ntrees_list="200
500
700
1000"

for min_node_size in ${min_node_size_list}
do
    for ntrees in ${ntrees_list}
    do
	sed "s/500/${ntrees}/g" run_config.conf | sed "s/15/${min_node_size}/g" >> run_config_ntrees_${ntrees}_minnodesize_${min_node_size}.conf
    done
done
