TEST_DIR=/media/markp/sdb/fio_test/
sudo fio --name=write_throughput \
	--directory=$TEST_DIR \
	--numjobs=1 \
	--size=10G \
	--time_based \
	--runtime=30s \
	--ramp_time=8s \
	--ioengine=libaio \
	--direct=1 \
	--verify=0 \
	--bs=4k \
	--iodepth=64 \
	--rw=randwrite \
	--fsync=1 \
	--log_avg_msec 0 \
	--group_reporting=1 \
	--write_bw_log=out --write_lat_log=out --write_iops_log=out \
	--per_job_logs=0 \
	--randrepeat=1

espeak "done"

