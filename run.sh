#make
#./raytrace 1920 1080 24 3
#./raytrace 1024 768 24 3
#time ./raytrace 23040 12810 160 8 10
time ./raytrace 800 600 12 8 10 100
cd ./render_output
./generate_movie.sh
