prefix=vib_5hz
for i in {1..10}
do
./run.sh

fio_generate_plots $prefix
mv $prefix-lat.svg fig/$prefix\_$i.svg
mv out_lat.log fig/$prefix\_lat_$i.csv
mv out_bw.log fig/$prefix\_bw_$i.csv

espeak "$i"
done
