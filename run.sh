#make
#./raytrace 1920 1080 24 3
#./raytrace 1024 768 24 3
#time ./raytrace 23040 12810 160 8 10
time ./raytrace 1280 720 80 8 10 100
cd ./render_output
./generate_movie.sh
